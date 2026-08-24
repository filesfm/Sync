#include "messaging.h"
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>

// API base URL
static const QString lockAPIEndpoint = "http://api.files.fm/api/file/lock.php";

Messaging::Messaging()
    : _lastCheckTmst(QDateTime::currentSecsSinceEpoch())
{
    connect(&_pollTimer, &QTimer::timeout, this, &Messaging::pollUnlockMessages);
}

void Messaging::setUserAndToken(const QString &user, const QString &token)
{
    _user = user;
    _token = token;

    // Stop polling if credentials are being cleared (empty user or token)
    if (_user.isEmpty() || _token.isEmpty()) {
        qInfo() << "Credentials cleared, stopping unlock message polling";
        stopUnlockMessagePolling();
    }
}

void Messaging::sendUnlockRequest(const QStringList &filePaths)
{
    if (_user.isEmpty() || _token.isEmpty()) {
        return;
    }
    // Construct the unlock request URL
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QUrl url(lockAPIEndpoint);
    QUrlQuery query;
    query.addQueryItem("action", "request_unlock_file_to_user");
    query.addQueryItem("user", _user);
    query.addQueryItem("user_access_token", _token);
    for (const QString &filePath : filePaths) {
        query.addQueryItem("file_paths[]", filePath);
    }
    url.setQuery(query);
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, manager]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Unlock request failed:" << reply->errorString();
        } else {
            qDebug() << "Unlock request sent successfully";
        }
        manager->deleteLater();
    });
}

void Messaging::startUnlockMessagePolling()
{
    _pollingEnabled = true;
    _pollTimer.start(10000); // Poll every 10 seconds
    pollUnlockMessages(); // initial immediate poll
}

void Messaging::stopUnlockMessagePolling()
{
    _pollingEnabled = false;
    _pollTimer.stop();
    qInfo() << "Stopped unlock message polling";
}

void Messaging::pollUnlockMessages()
{
    // Don't poll if polling is disabled or credentials are missing
    if (!_pollingEnabled || _user.isEmpty() || _token.isEmpty()) {
        qWarning() << "Polling is disabled or credentials are missing";
        return;
    }
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QUrl url(lockAPIEndpoint);
    QUrlQuery query;
    query.addQueryItem("action", "get_unlock_messages");
    query.addQueryItem("user", _user);
    query.addQueryItem("user_access_token", _token);
    query.addQueryItem("last_check_tmst", QString::number(_lastCheckTmst));
    url.setQuery(query);
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        handleNetworkReply(reply);
        manager->deleteLater();
    });
}

void Messaging::handleNetworkReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Unlock message polling failed:" << reply->errorString();
        return;
    }
    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isObject()) {
        qWarning() << "Invalid unlock message response";
        return;
    }
    auto obj = doc.object();
    if (obj.value("success").toBool()) {
        QJsonObject unlock = obj.value("unlock_messages").toObject();
        QJsonArray requests = unlock.value("requests").toArray();
        QJsonArray responses = unlock.value("responses").toArray();
        _lastCheckTmst = obj.value("check_tmst").toVariant().toLongLong();
        for (const QJsonValue &reqVal : requests) {
            QJsonObject req = reqVal.toObject();
            QString username = req.value("user").toObject().value("username").toString();
            QString filename = req.value("file").toObject().value("name").toString();
            QString filepath = req.value("file").toObject().value("path").toString();
            emit unlockRequestReceived(username, filename, filepath);
            QString html = tr("User %1 requested to unlock file:<br><b>%2</b><br><span style='color:grey;font-size:small;'>%3</span>")
                               .arg(username, filename, filepath.toHtmlEscaped());
            html += tr("<br><br>To unlock the file, close it if you currently have opened it or unlock it with the contextmenu Unlock button.");
            QMessageBox *msgBox = new QMessageBox;
            msgBox->setAttribute(Qt::WA_DeleteOnClose);
            msgBox->setWindowTitle(tr("File Unlock Request"));
            msgBox->setTextFormat(Qt::RichText);
            msgBox->setText(html);
            msgBox->setWindowFlag(Qt::WindowStaysOnTopHint, true);
            msgBox->show();
            msgBox->raise();
        }
        // Show notification for unlock responses
        for (const QJsonValue &respVal : responses) {
            QJsonObject resp = respVal.toObject();
            QString username = resp.value("user").toObject().value("username").toString();
            QString filename = resp.value("file").toObject().value("name").toString();
            QString filepath = resp.value("file").toObject().value("path").toString();
            qWarning() << "Response received by account" << _user << "in Messaging instance" << this;
            emit unlockResponseReceived(filepath);
            QString html = tr("File <b>%1</b> has been unlocked by %2.<br><span style='color:grey;font-size:small;'>%3</span>")
                               .arg(filename, username, filepath.toHtmlEscaped());
            html += tr("<br><br>Please <b>close and reopen</b> the file to edit it.");
            QMessageBox *msgBox = new QMessageBox;
            msgBox->setAttribute(Qt::WA_DeleteOnClose);
            msgBox->setWindowTitle(tr("File Unlocked"));
            msgBox->setTextFormat(Qt::RichText);
            msgBox->setText(html);
            msgBox->setWindowFlag(Qt::WindowStaysOnTopHint, true);
            msgBox->show();
            msgBox->raise();
            msgBox->activateWindow();
        }
    }
}
