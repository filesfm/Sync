#include "lockwatcher.h"
#include "filehandles.h"

#include <QDebug>
#include <QThread>
#include <QSet>

LockWatcher::LockWatcher(const QString &path, QObject *parent)
    : _path(path)
    , QObject(parent)
{
}

void LockWatcher::run()
{
    watch();
}

void LockWatcher::watch()
{
    QSet<QString> previouslyLockedFiles;

    while (!QThread::currentThread()->isInterruptionRequested()) {
        QSet<QString> currentlyLockedFiles;
        try {
            currentlyLockedFiles = getFileHandles(_path);
        } catch (const std::exception &e) {
            qWarning() << "Exception while getting file handles:" << e.what();
            QThread::sleep(10);
            continue;
        } catch (...) {
            qWarning() << "Unknown exception while getting file handles.";
            QThread::sleep(10);
            continue;
        }

        const auto newLocks = currentlyLockedFiles - previouslyLockedFiles;
        const auto unlocked = previouslyLockedFiles - currentlyLockedFiles;

        for (const QString &file : newLocks) {
            qWarning() << "Locked file:" << file;
            emit fileLocked(file);
        }

        for (const QString &file : unlocked) {
            emit fileUnlocked(file);
            qWarning() << "Unlocked file:" << file;
        }

        previouslyLockedFiles = currentlyLockedFiles;
        QThread::sleep(10);
    }
}
