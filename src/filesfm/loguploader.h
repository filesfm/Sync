#ifndef FILESFM_LOGUPLOADER_H
#define FILESFM_LOGUPLOADER_H

#include <QObject>
#include <QString>
#include <QStringList>

class LogUploader : public QObject
{
    Q_OBJECT
public:
    explicit LogUploader(QObject *parent = nullptr);

    // Uploads each file in filePaths one at a time. The server only accepts
    // one file per request, so multiple log files are sent as separate requests.
    void uploadLogFiles(const QStringList &filePaths, const QString &user, const QString &pass);

signals:
    void uploadFinished(bool success, const QString &message);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void fileUploadStarted(int index, int total, const QString &fileName);

private:
    void uploadNext();

    QStringList _pendingFiles;
    QStringList _failedFiles;
    QString _user;
    QString _pass;
    int _totalFiles = 0;
};

#endif // FILESFM_LOGUPLOADER_H
