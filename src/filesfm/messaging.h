#ifndef FILESFM_NEW_MESSAGING_H
#define FILESFM_NEW_MESSAGING_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QNetworkReply>

class Messaging : public QObject
{
    Q_OBJECT
public:
    // Construct with user and token
    explicit Messaging();
    void setUserAndToken(const QString &user, const QString &token);
    void startUnlockMessagePolling();
    void stopUnlockMessagePolling();

    // Sends an unlock request for the specified file paths
    void sendUnlockRequest(const QStringList &filePaths);

signals:
    // Signal emitted when an unlock request is received
    void unlockRequestReceived(const QString &username, const QString &filename, const QString &filepath);
    // Signal emitted when an unlock response is received for a file
    void unlockResponseReceived(const QString &filepath);

private slots:
    void pollUnlockMessages();
    void handleNetworkReply(QNetworkReply *reply);

private:
    QString _user;
    QString _token;
    QTimer _pollTimer;
    qint64 _lastCheckTmst;
    bool _pollingEnabled = false;
};

#endif // FILESFM_NEW_MESSAGING_H
