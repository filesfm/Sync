#ifndef LOCKWATCHER_H
#define LOCKWATCHER_H

#include <QObject>
#include <QString>

class LockWatcher : public QObject
{
    Q_OBJECT
public:
    explicit LockWatcher(const QString &path, QObject *parent = nullptr);

public slots:
    void run();

signals:
    void fileLocked(const QString &path);
    void fileUnlocked(const QString &path);

private:
    void watch();
    QString _path;
};

#endif // LOCKWATCHER_H
