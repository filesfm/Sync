/*
 * Copyright (C) by Christian Kamm <mail@ckamm.de>
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

#include "guiutility.h"

#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include <QHash>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QUrlQuery>

#include "common/asserts.h"
using namespace OCC;

Q_LOGGING_CATEGORY(lcUtility, "nextcloud.gui.utility", QtInfoMsg)

class Utility::ScopedHostEnvironment::Private
{
public:
#ifdef Q_OS_LINUX
    QMutexLocker<QMutex> locker;
    QHash<QByteArray, QByteArray> savedVars;
    explicit Private(QMutex &mutex)
        : locker(&mutex)
    {
    }
#endif
};

namespace {
// This can be locked from multiple threads (e.g. EditLocallyJob runs openUrlSafely() via
// QtConcurrent::run()), and mutating process environment variables concurrently is not
// thread-safe, so every ScopedHostEnvironment serializes on this.
QMutex hostEnvironmentMutex;
}

Utility::ScopedHostEnvironment::ScopedHostEnvironment()
#ifdef Q_OS_LINUX
    : _d(new Private(hostEnvironmentMutex))
#else
    : _d(nullptr)
#endif
{
#ifdef Q_OS_LINUX
    static const char *const pollutedVars[] = {"LD_LIBRARY_PATH", "QT_PLUGIN_PATH", "QML2_IMPORT_PATH"};
    for (const auto *name : pollutedVars) {
        if (qEnvironmentVariableIsSet(name)) {
            _d->savedVars.insert(name, qgetenv(name));
            qunsetenv(name);
        }
    }
#endif
}

Utility::ScopedHostEnvironment::~ScopedHostEnvironment()
{
#ifdef Q_OS_LINUX
    for (auto it = _d->savedVars.constBegin(); it != _d->savedVars.constEnd(); ++it) {
        qputenv(it.key().constData(), it.value());
    }
    delete _d;
#endif
}

bool Utility::openUrlSafely(const QUrl &url)
{
    // See ScopedHostEnvironment's documentation: an AppImage build needs
    // LD_LIBRARY_PATH/QT_PLUGIN_PATH/QML2_IMPORT_PATH for itself, but QDesktopServices::openUrl()
    // forks+execs xdg-open synchronously on Linux (when no desktop portal is in use), which must
    // not inherit them.
    ScopedHostEnvironment hostEnv;
    return QDesktopServices::openUrl(url);
}

bool Utility::openBrowser(const QUrl &url, QWidget *errorWidgetParent)
{
    const QStringList allowedUrlSchemes = {
        "http",
        "https",
    };

    if (!allowedUrlSchemes.contains(url.scheme())) {
        qCWarning(lcUtility) << "URL format is not supported, or it has been compromised for:" << url.toString();
        return false;
    }

    if (!openUrlSafely(url)) {
        if (errorWidgetParent) {
            QMessageBox::warning(
                errorWidgetParent,
                QCoreApplication::translate("utility", "Could not open browser"),
                QCoreApplication::translate("utility",
                    "There was an error when launching the browser to go to "
                    "URL %1. Maybe no default browser is configured?")
                    .arg(url.toString()));
        }
        qCWarning(lcUtility) << "QDesktopServices::openUrl failed for" << url;
        return false;
    }
    return true;
}

bool Utility::openEmailComposer(const QString &subject, const QString &body, QWidget *errorWidgetParent)
{
    QUrl url(QLatin1String("mailto:"));
    QUrlQuery query;
    query.setQueryItems({ { QLatin1String("subject"), subject },
        { QLatin1String("body"), body } });
    url.setQuery(query);

    if (!openUrlSafely(url)) {
        if (errorWidgetParent) {
            QMessageBox::warning(
                errorWidgetParent,
                QCoreApplication::translate("utility", "Could not open email client"),
                QCoreApplication::translate("utility",
                    "There was an error when launching the email client to "
                    "create a new message. Maybe no default email client is "
                    "configured?"));
        }
        qCWarning(lcUtility) << "QDesktopServices::openUrl failed for" << url;
        return false;
    }
    return true;
}

QString Utility::vfsCurrentAvailabilityText(VfsItemAvailability availability)
{
    switch(availability) {
    case VfsItemAvailability::AlwaysLocal:
        return QCoreApplication::translate("utility", "Always available locally");
    case VfsItemAvailability::AllHydrated:
        return QCoreApplication::translate("utility", "Currently available locally");
    case VfsItemAvailability::Mixed:
        return QCoreApplication::translate("utility", "Some available online only");
    case VfsItemAvailability::AllDehydrated:
    case VfsItemAvailability::OnlineOnly:
        return QCoreApplication::translate("utility", "Available online only");
    }
    Q_UNREACHABLE();
}

QString Utility::vfsPinActionText()
{
    return QCoreApplication::translate("utility", "Make always available locally");
}

QString Utility::vfsFreeSpaceActionText()
{
    return QCoreApplication::translate("utility", "Free up local space");
}
