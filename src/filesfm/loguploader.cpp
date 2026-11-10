#include "loguploader.h"

#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QLoggingCategory>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

Q_LOGGING_CATEGORY(lcLogUploader, "nextcloud.sync.loguploader", QtInfoMsg)

static const QString logUploadEndpoint = QStringLiteral("https://files.fm/new/api/log/upload.php");

LogUploader::LogUploader(QObject *parent)
    : QObject(parent)
{
}

void LogUploader::uploadLogFiles(const QStringList &filePaths, const QString &user, const QString &pass)
{
    _pendingFiles = filePaths;
    _failedFiles.clear();
    _user = user;
    _pass = pass;
    _totalFiles = filePaths.size();
    uploadNext();
}

void LogUploader::uploadNext()
{
    if (_pendingFiles.isEmpty()) {
        if (_failedFiles.isEmpty()) {
            emit uploadFinished(true, tr("Logs uploaded successfully"));
        } else {
            emit uploadFinished(false, tr("Failed to upload:\n%1").arg(_failedFiles.join(QStringLiteral("\n"))));
        }
        return;
    }

    const auto filePath = _pendingFiles.takeFirst();
    const auto currentIndex = _totalFiles - _pendingFiles.size();
    emit fileUploadStarted(currentIndex, _totalFiles, QFileInfo(filePath).fileName());

    auto *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        delete file;
        _failedFiles.append(QFileInfo(filePath).fileName());
        uploadNext();
        return;
    }

    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart userPart;
    userPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QStringLiteral("form-data; name=\"user\"")));
    userPart.setBody(_user.toUtf8());
    multiPart->append(userPart);

    QHttpPart passPart;
    passPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QStringLiteral("form-data; name=\"pass\"")));
    passPart.setBody(_pass.toUtf8());
    multiPart->append(passPart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant(QStringLiteral("form-data; name=\"uploads[]\"; filename=\"%1\"").arg(QFileInfo(filePath).fileName())));
    file->setParent(multiPart);
    filePart.setBodyDevice(file);
    multiPart->append(filePart);

    auto *manager = new QNetworkAccessManager();
    QNetworkRequest request{QUrl(logUploadEndpoint)};

    QNetworkReply *reply = manager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::uploadProgress, this, &LogUploader::uploadProgress);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager, filePath]() {
        const auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto body = reply->readAll().trimmed();
        const auto networkError = reply->error();

        if (networkError != QNetworkReply::NoError || body != QByteArrayLiteral("OK")) {
            const auto reason = networkError != QNetworkReply::NoError
                ? reply->errorString()
                : (body.isEmpty() ? tr("empty response") : QString::fromUtf8(body.left(200)));
            qCWarning(lcLogUploader) << "Upload failed for" << filePath << "HTTP status" << statusCode << "reason:" << reason;
            _failedFiles.append(tr("%1 (HTTP %2: %3)").arg(QFileInfo(filePath).fileName()).arg(statusCode).arg(reason));
        } else {
            qCInfo(lcLogUploader) << "Uploaded" << filePath;
        }

        reply->deleteLater();
        manager->deleteLater();
        uploadNext();
    });
}
