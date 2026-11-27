/*
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "updater/appimageupdater.h"

#include "configfile.h"
#include "theme.h"
#include "common/utility.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QStyle>
#include <QVBoxLayout>

namespace OCC {

namespace {
const auto updateAvailableC = QStringLiteral("Updater/updateAvailable");
const auto updateTargetVersionC = QStringLiteral("Updater/updateTargetVersion");
const auto updateTargetVersionStringC = QStringLiteral("Updater/updateTargetVersionString");
const auto autoUpdateAttemptedC = QStringLiteral("Updater/autoUpdateAttempted");
const auto versionPatternC = QStringLiteral("^[0-9]+(\\.[0-9]+){0,3}$");
}

AppImageUpdater::AppImageUpdater(const QUrl &url)
    : OCUpdater(url)
{
}

QString AppImageUpdater::runningAppImagePath()
{
    return QString::fromLocal8Bit(qgetenv("APPIMAGE"));
}

void AppImageUpdater::versionInfoArrived(const UpdateInfo &info)
{
    qCInfo(lcUpdater) << "Version info arrived:"
                       << "Your version:" << Helper::currentVersionToInt()
                       << "Available version:" << info.version()
                       << "Available version string:" << info.versionString()
                       << "Web url:" << info.web()
                       << "Download url:" << info.downloadUrl();

    const auto appImagePath = runningAppImagePath();
    if (appImagePath.isEmpty()) {
        qCWarning(lcUpdater) << "APPIMAGE is not set, skipping AppImage update";
        setDownloadState(UpToDate);
        return;
    }

    if (info.version().isEmpty()) {
        setDownloadState(UpToDate);
        return;
    }

    static const QRegularExpression versionPattern(versionPatternC);
    if (!versionPattern.match(info.version()).hasMatch()) {
        qCWarning(lcUpdater) << "Update feed returned a malformed version string, ignoring:" << info.version();
        setDownloadState(UpToDate);
        return;
    }

    if (Helper::currentVersionToInt() >= Helper::stringVersionToInt(info.version())) {
        qCInfo(lcUpdater) << "Client is on latest version!";
        setDownloadState(UpToDate);
        return;
    }

    const auto url = info.downloadUrl();
    if (url.isEmpty()) {
        showNoUrlDialog(info);
        return;
    }

    const QFileInfo appImageInfo(appImagePath);
    _targetFile = appImageInfo.absolutePath() + QLatin1String("/files_fm_sync-") + info.version() + QLatin1String(".AppImage");

    if (QFile::exists(_targetFile)) {
        setDownloadState(DownloadComplete);
        return;
    }

    auto request = QNetworkRequest(QUrl(url));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = qnam()->get(request);

    _file.reset(new QTemporaryFile(appImageInfo.absolutePath() + QLatin1String("/.files_fm_sync-update.XXXXXX")));
    _file->setAutoRemove(true);
    if (!_file->open()) {
        qCWarning(lcUpdater) << "Could not create a temporary file for the AppImage download";
        reply->abort();
        setDownloadState(DownloadFailed);
        return;
    }

    connect(reply, &QIODevice::readyRead, this, &AppImageUpdater::slotWriteFile);
    connect(reply, &QNetworkReply::finished, this, &AppImageUpdater::slotDownloadFinished);
    setDownloadState(Downloading);
}

void AppImageUpdater::slotWriteFile()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (_file->isOpen()) {
        _file->write(reply->readAll());
    }
}

void AppImageUpdater::slotDownloadFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(lcUpdater) << "AppImage download failed:" << reply->errorString();
        _file->remove();
        setDownloadState(DownloadFailed);
        return;
    }

    _file->close();

    const auto fail = [this](const QString &reason) {
        qCWarning(lcUpdater) << "Rejecting downloaded AppImage update:" << reason;
        _file->remove();
        setDownloadState(DownloadFailed);
    };

    const auto fileSize = QFileInfo(_file->fileName()).size();
    if (fileSize <= 0) {
        fail(QStringLiteral("empty download"));
        return;
    }

    const auto contentLength = reply->header(QNetworkRequest::ContentLengthHeader);
    if (contentLength.isValid() && contentLength.toLongLong() != fileSize) {
        fail(QStringLiteral("size mismatch (truncated download)"));
        return;
    }

    QFile downloaded(_file->fileName());
    if (!downloaded.open(QIODevice::ReadOnly)) {
        fail(QStringLiteral("could not reopen downloaded file"));
        return;
    }

    if (downloaded.read(4) != QByteArrayLiteral("\x7f""ELF")) {
        downloaded.close();
        fail(QStringLiteral("not an ELF binary"));
        return;
    }
    downloaded.close();

    if (!_file->setPermissions(_file->permissions() | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther)) {
        fail(QStringLiteral("could not make the downloaded file executable"));
        return;
    }

    _file->setAutoRemove(false);
    if (!_file->rename(_targetFile)) {
        qCWarning(lcUpdater) << "Could not move downloaded update into place:" << _file->errorString();
        _file->remove();
        setDownloadState(DownloadFailed);
        return;
    }

    ConfigFile cfg;
    QSettings settings(cfg.configFile(), QSettings::IniFormat);

    QFile oldTargetFile(settings.value(updateAvailableC).toString());
    if (oldTargetFile.fileName() != _targetFile && oldTargetFile.exists()) {
        oldTargetFile.remove();
    }

    qCInfo(lcUpdater) << "Downloaded and verified AppImage update to" << _targetFile;
    settings.setValue(updateTargetVersionC, updateInfo().version());
    settings.setValue(updateTargetVersionStringC, updateInfo().versionString());
    settings.setValue(updateAvailableC, _targetFile);
    setDownloadState(DownloadComplete);
}

void AppImageUpdater::wipeUpdateData()
{
    ConfigFile cfg;
    QSettings settings(cfg.configFile(), QSettings::IniFormat);
    const auto updateFileName = settings.value(updateAvailableC).toString();
    if (!updateFileName.isEmpty()) {
        QFile::remove(updateFileName);
    }
    settings.remove(updateAvailableC);
    settings.remove(updateTargetVersionC);
    settings.remove(updateTargetVersionStringC);
    settings.remove(autoUpdateAttemptedC);
}

bool AppImageUpdater::handleStartup()
{
    ConfigFile cfg;
    QSettings settings(cfg.configFile(), QSettings::IniFormat);

    if (cfg.skipUpdateCheck() || !cfg.autoUpdateCheck()) {
        qCInfo(lcUpdater) << "Skipping installation of update due to config settings";
        return false;
    }

    const auto updateFileName = settings.value(updateAvailableC).toString();
    if (!updateFileName.isEmpty() && QFile::exists(updateFileName)) {
        qCInfo(lcUpdater) << "An updater file is available";
        if (settings.value(autoUpdateAttemptedC, false).toBool()) {
            if (updateSucceeded()) {
                qCInfo(lcUpdater) << "The requested update attempt has succeeded" << Helper::currentVersionToInt();
                wipeUpdateData();
                return false;
            }
            qCInfo(lcUpdater) << "The requested update attempt has failed" << settings.value(updateTargetVersionC).toString();
            showUpdateErrorDialog(settings.value(updateTargetVersionStringC).toString());
            return false;
        }
        qCInfo(lcUpdater) << "Triggering an update";
        return performUpdate();
    }
    return false;
}

void AppImageUpdater::showNoUrlDialog(const UpdateInfo &info)
{
    auto *msgBox = new QDialog;
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowFlags(msgBox->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QIcon infoIcon = msgBox->style()->standardIcon(QStyle::SP_MessageBoxInformation);
    int iconSize = msgBox->style()->pixelMetric(QStyle::PM_MessageBoxIconSize);

    msgBox->setWindowIcon(infoIcon);

    auto *layout = new QVBoxLayout(msgBox);
    auto *hlayout = new QHBoxLayout;
    layout->addLayout(hlayout);

    msgBox->setWindowTitle(tr("New Version Available"));

    auto *ico = new QLabel;
    ico->setFixedSize(iconSize, iconSize);
    ico->setPixmap(infoIcon.pixmap(iconSize));
    auto *lbl = new QLabel;
    QString txt = tr("<p>A new version of the %1 Client is available.</p>"
                      "<p><b>%2</b> is available for download. The installed version is %3.</p>")
                      .arg(Utility::escape(Theme::instance()->appNameGUI()),
                          Utility::escape(info.versionString()), Utility::escape(clientVersion()));

    lbl->setText(txt);
    lbl->setTextFormat(Qt::RichText);
    lbl->setWordWrap(true);

    hlayout->addWidget(ico);
    hlayout->addWidget(lbl);

    auto *bb = new QDialogButtonBox;
    QPushButton *reject = bb->addButton(tr("Skip this time"), QDialogButtonBox::AcceptRole);
    QPushButton *getupdate = bb->addButton(tr("Get update"), QDialogButtonBox::AcceptRole);

    connect(reject, &QAbstractButton::clicked, msgBox, &QDialog::reject);
    connect(getupdate, &QAbstractButton::clicked, msgBox, &QDialog::accept);

    connect(getupdate, &QAbstractButton::clicked, this, &AppImageUpdater::slotOpenUpdateUrl);

    layout->addWidget(bb);

    msgBox->open();
}

void AppImageUpdater::showUpdateErrorDialog(const QString &targetVersion)
{
    auto msgBox = new QDialog;
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowFlags(msgBox->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QIcon infoIcon = msgBox->style()->standardIcon(QStyle::SP_MessageBoxInformation);
    int iconSize = msgBox->style()->pixelMetric(QStyle::PM_MessageBoxIconSize);

    msgBox->setWindowIcon(infoIcon);

    auto layout = new QVBoxLayout(msgBox);
    auto hlayout = new QHBoxLayout;
    layout->addLayout(hlayout);

    msgBox->setWindowTitle(tr("Update Failed"));

    auto ico = new QLabel;
    ico->setFixedSize(iconSize, iconSize);
    ico->setPixmap(infoIcon.pixmap(iconSize));
    auto lbl = new QLabel;
    QString txt = tr("<p>A new version of the %1 Client is available but the updating process failed.</p>"
                      "<p><b>%2</b> has been downloaded. The installed version is %3. If you confirm restart and update, the client will restart to complete the installation.</p>")
                      .arg(Utility::escape(Theme::instance()->appNameGUI()),
                          Utility::escape(targetVersion), Utility::escape(clientVersion()));

    lbl->setText(txt);
    lbl->setTextFormat(Qt::RichText);
    lbl->setWordWrap(true);

    hlayout->addWidget(ico);
    hlayout->addWidget(lbl);

    auto bb = new QDialogButtonBox;
    auto askagain = bb->addButton(tr("Ask again later"), QDialogButtonBox::ResetRole);
    auto retry = bb->addButton(tr("Restart and update"), QDialogButtonBox::AcceptRole);
    auto getupdate = bb->addButton(tr("Update manually"), QDialogButtonBox::AcceptRole);

    connect(askagain, &QAbstractButton::clicked, msgBox, &QDialog::reject);
    connect(retry, &QAbstractButton::clicked, msgBox, &QDialog::accept);
    connect(getupdate, &QAbstractButton::clicked, msgBox, &QDialog::accept);

    connect(retry, &QAbstractButton::clicked, this, [this]() {
        slotStartInstaller();
    });
    connect(getupdate, &QAbstractButton::clicked, this, [this]() {
        slotOpenUpdateUrl();
    });

    layout->addWidget(bb);

    msgBox->open();
}

} // namespace OCC
