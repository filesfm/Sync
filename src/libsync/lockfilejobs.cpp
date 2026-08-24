/*lockfile
 * Copyright (C) by Matthieu Gallien <matthieu.gallien@nextcloud.com>
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

#include "lockfilejobs.h"

#include "account.h"
#include "common/syncjournaldb.h"
#include "filesystem.h"
#include "creds/httpcredentials.h"
#include "creds/abstractcredentials.h"

#include <QLoggingCategory>
#include <QXmlStreamReader>
#include <QJsonArray>

namespace OCC {

Q_LOGGING_CATEGORY(lcLockFileJob, "nextcloud.sync.networkjob.lockfile", QtInfoMsg)

LockFileJob::LockFileJob(const AccountPtr account,
                         SyncJournalDb* const journal,
                         const QString &path,
                         const QString &remoteSyncPathWithTrailingSlash,
                         const QString &localSyncPath,
                         const QString &etag,
                         const SyncFileItem::LockStatus requestedLockState,
                         const SyncFileItem::LockOwnerType lockOwnerType,
                         QObject *parent)
    : AbstractNetworkJob(account, path, parent)
    , _currentAttempt(0) // Initialize _currentAttempt here
    , _journal(journal)
    , _requestedLockState(requestedLockState)
    , _requestedLockOwnerType(lockOwnerType)
    , _remoteSyncPathWithTrailingSlash(remoteSyncPathWithTrailingSlash)
    , _localSyncPath(localSyncPath)
    , _existingEtag(etag)
{
    if (!_localSyncPath.endsWith(QLatin1Char('/'))) {
        _localSyncPath.append(QLatin1Char('/'));
    }
}

void LockFileJob::start()
{
    qCInfo(lcLockFileJob()) << "start with path:" << path()
                            << "lock state:" <<  _requestedLockState
                            << "lock owner type:" << _requestedLockOwnerType
                            << "attempt:" << _currentAttempt << "/" << MAX_RETRIES;

    QNetworkRequest request;

    QByteArray action;
    switch(_requestedLockState)
    {
    case SyncFileItem::LockStatus::LockedItem:
    {
        action = "lock_file";
        break;
    }
    case SyncFileItem::LockStatus::UnlockedItem:
        action = "unlock_file";
        break;
    }

    // Prevent Basic Auth and use the appPassword/user_access_token instead
    request.setAttribute(HttpCredentials::DontAddCredentialsAttribute, true);

    QUrl url("https://api.files.fm/api/file/lock.php");
    QUrlQuery query;
    query.addQueryItem("user", _account->credentials()->user());
    query.addQueryItem("user_access_token", _account->credentials()->password());
    query.addQueryItem("action", action);
    query.addQueryItem("file_paths[]", path());
    url.setQuery(query);
    sendRequest("GET", url, request);

    AbstractNetworkJob::start();
}

bool LockFileJob::finished()
{
    if (reply()->error() != QNetworkReply::NoError) {
        if (_currentAttempt < MAX_RETRIES) {
            _currentAttempt++; // Increment attempt counter
            qCInfo(lcLockFileJob) << "Retrying LockFileJob for path:" << path() << "attempt:" << _currentAttempt + 1;
            retry();
            return false; // Job is not finished yet
        }

        qCWarning(lcLockFileJob) << "LockFileJob finished with network error:" << reply()->error()
                                 << reply()->errorString() << path() << "attempt:" << _currentAttempt;

        // All retries exhausted, emit the error
        qCWarning(lcLockFileJob) << "LockFileJob failed after" << MAX_RETRIES << "attempts for path:" << path();
        const auto httpErrorCode = reply()->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpErrorCode == LOCKED_HTTP_ERROR_CODE) {
            const auto record = handleReply();
            if (static_cast<SyncFileItem::LockOwnerType>(record._lockstate._lockOwnerType) == SyncFileItem::LockOwnerType::UserLock) {
                Q_EMIT finishedWithError(httpErrorCode, {}, record._lockstate._lockOwnerDisplayName);
            } else {
                Q_EMIT finishedWithError(httpErrorCode, {}, record._lockstate._lockEditorApp);
            }
        } else if (httpErrorCode == PRECONDITION_FAILED_ERROR_CODE) {
            const auto record = handleReply();
            if (_requestedLockState == SyncFileItem::LockStatus::UnlockedItem && !record._lockstate._locked) {
                Q_EMIT finishedWithoutError();
            } else {
                Q_EMIT finishedWithError(httpErrorCode, reply()->errorString(), {});
            }
        } else {
            Q_EMIT finishedWithError(httpErrorCode, reply()->errorString(), {});
        }
    } else {
        qCInfo(lcLockFileJob()) << "LockFileJob success for path:" << path() << "after" << _currentAttempt << "attempts";
        _currentAttempt = 0; // Reset retry counter on success
        handleReply();
        Q_EMIT finishedWithoutError();
    }
    return true;
}

void LockFileJob::setFileRecordLocked(SyncJournalFileRecord &record) const
{
    record._lockstate._locked = (_lockStatus == SyncFileItem::LockStatus::LockedItem);
    record._lockstate._lockOwnerType = static_cast<int>(_lockOwnerType);
    record._lockstate._lockOwnerDisplayName = _userDisplayName;
    record._lockstate._lockOwnerId = _userId;
    record._lockstate._lockEditorApp = _editorName;
    record._lockstate._lockTime = _lockTime;
    record._lockstate._lockTimeout = _lockTimeout;
    record._lockstate._lockToken = _lockToken;
    if (!_etag.isEmpty()) {
        record._etag = _etag;
    }
}

void LockFileJob::resetState()
{
    _lockStatus = SyncFileItem::LockStatus::UnlockedItem;
    _lockOwnerType = SyncFileItem::LockOwnerType::UserLock;
    _userDisplayName.clear();
    _editorName.clear();
    _userId.clear();
    _lockTime = 0;
    _lockTimeout = 0;
    _lockToken.clear();
}

SyncJournalFileRecord LockFileJob::handleReply()
{
    const auto responseData = reply()->readAll();
    qInfo() << "Response data:" << responseData;

    resetState();

    SyncJournalFileRecord record;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(lcLockFileJob) << "Failed to parse JSON response:" << parseError.errorString();
        return record;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray filesArray = rootObj.value("files").toArray();
    if (filesArray.isEmpty()) {
        qCWarning(lcLockFileJob) << "No files in response.";
        // Even if no files array, we should still try to update the journal based on the request.
        // This path indicates a server-side issue or unexpected response format.
        // We will proceed to try and retrieve the existing record and apply requested state.
    }

    QJsonObject fileObj;
    QJsonObject fileLockData;
    bool success = rootObj.value("success").toBool(); // Overall API call success
    bool fileSuccess = false; // Individual file operation success

    if (!filesArray.isEmpty()) {
        fileObj = filesArray.at(0).toObject();
        fileSuccess = fileObj.value("success").toBool(); // Specific file operation success
        fileLockData = fileObj.value("file_lock_data").toObject();
    }

    // Populate member variables based on response, prioritizing overall success then file-specific success
    if (success && fileSuccess) {
        // Successful lock/unlock operation
        if (_requestedLockState == SyncFileItem::LockStatus::LockedItem) {
            _lockStatus = SyncFileItem::LockStatus::LockedItem;
            _lockOwnerType = static_cast<SyncFileItem::LockOwnerType>(fileLockData.value("lock_owner_type").toInt(static_cast<int>(SyncFileItem::LockOwnerType::UserLock)));
            _userDisplayName = _account->credentials()->user();
            _userId = _account->credentials()->user();
            // _editorName = fileLockData.value("editor").toString();
            _lockTime = QDateTime::currentSecsSinceEpoch();
            _lockTimeout = QDateTime::fromString(fileLockData.value("date_timeout").toString(), "yyyy-MM-dd HH:mm").toSecsSinceEpoch();
            // _lockToken = fileLockData.value("lock_token").toString();
            // _etag = fileLockData.value("etag").toString().toUtf8();

            // Set file as read-only on successful lock
            // const auto relativePathInDb = path().mid(_remoteSyncPathWithTrailingSlash.size());
            // FileSystem::setFileReadOnly(_localSyncPath + relativePathInDb, true);

        } else if (_requestedLockState == SyncFileItem::LockStatus::UnlockedItem) {
            _lockStatus = SyncFileItem::LockStatus::UnlockedItem;
            // Set file as writable on successful unlock
            // const auto relativePathInDb = path().mid(_remoteSyncPathWithTrailingSlash.size());
            // FileSystem::setFileReadOnly(_localSyncPath + relativePathInDb, false);
        }
    } else { // Overall API call or specific file operation was not successful
        qCWarning(lcLockFileJob) << "Server response was not successful or file operation failed.";
        // Check if file is locked by another user even if the requested operation failed
        if (fileLockData.value("is_locked").toBool() && !fileLockData.value("locked_by_me").toBool()) {
            _lockStatus = SyncFileItem::LockStatus::LockedItem;
            _lockOwnerType = static_cast<SyncFileItem::LockOwnerType>(fileLockData.value("lock_owner_type").toInt(static_cast<int>(SyncFileItem::LockOwnerType::UserLock)));
            _userDisplayName = fileLockData.value("user").toObject().value("username").toString();
            _userId = fileLockData.value("user").toObject().value("username").toString();
            // _userId = fileLockData.value("user").toObject().value("id").toString();
            // _editorName = fileLockData.value("editor").toString();
            _lockTime = QDateTime::currentSecsSinceEpoch();
            _lockTimeout = QDateTime::fromString(fileLockData.value("date_timeout").toString(), "yyyy-MM-dd HH:mm").toSecsSinceEpoch();
            // _lockToken = fileLockData.value("lock_token").toString();
            // _etag = fileLockData.value("etag").toString().toUtf8();

            // Mark file as readonly in filesystem if locked by another user
            // const auto relativePathInDb = path().mid(_remoteSyncPathWithTrailingSlash.size());
            // FileSystem::setFileReadOnly(_localSyncPath + relativePathInDb, true);

            QJsonObject fileError = fileObj.value("error").toObject();
            if (fileError.value("code").toInt() == 2002) {
                // Emit error for UI if locked by another user and this was the specific error code
                Q_EMIT finishedWithError(LOCKED_HTTP_ERROR_CODE, {}, _userDisplayName);
            }
        } else {
            // If not locked by another user and operation failed, assume unlocked state or error handling elsewhere
            _lockStatus = SyncFileItem::LockStatus::UnlockedItem; // Default to unlocked on failure if not explicitly locked by others
        }
    }

    const auto relativePathInDb = path().mid(_remoteSyncPathWithTrailingSlash.size());
    // Always try to retrieve the record and update it with the new state
    // This ensures the journal is consistent with the server's response, or the default unlocked state on failure.
    if (!_journal->getFileRecord(relativePathInDb, &record)) {
        qCWarning(lcLockFileJob) << "Could not retrieve file record for path:" << relativePathInDb << ", creating a new one.";
        // record._path = relativePathInDb;
    }

    setFileRecordLocked(record);

    const auto result = _journal->setFileRecord(record);
    if (!result) {
        qCWarning(lcLockFileJob) << "Error when setting the file record to the database" << record._path << result.error();
    }
    _journal->commit("lock file job");

    return record;
}

}
