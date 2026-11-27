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

#ifndef APPIMAGEUPDATER_H
#define APPIMAGEUPDATER_H

#include "updater/ocupdater.h"

#include <QScopedPointer>
#include <QTemporaryFile>

namespace OCC {

class AppImageUpdater : public OCUpdater
{
    Q_OBJECT
public:
    explicit AppImageUpdater(const QUrl &url);
    bool handleStartup() override;

private slots:
    void slotWriteFile();
    void slotDownloadFinished();

private:
    void versionInfoArrived(const UpdateInfo &info) override;
    void showNoUrlDialog(const UpdateInfo &info);
    void showUpdateErrorDialog(const QString &targetVersion);
    void wipeUpdateData();

    [[nodiscard]] static QString runningAppImagePath();

    QScopedPointer<QTemporaryFile> _file;
    QString _targetFile;
};

} // namespace OCC

#endif // APPIMAGEUPDATER_H
