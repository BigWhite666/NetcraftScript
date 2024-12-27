#pragma once

#include <QObject>
#include <QThread>
#include <QList>
#include <windows.h>
#include "dm/dmutils.h"

struct LoginAccount {
    QString account;
    QString password;
    int interval;
};

// 工作线程类
class LoginWorker : public QObject {
    Q_OBJECT
    
public:
    explicit LoginWorker(QObject* parent = nullptr);
    void setParams(const QList<LoginAccount>& accounts, const QString& gamePath);
    void stop();

public slots:
    void doWork();  // 执行任务

signals:
    void messageUpdated(const QString& message);
    void statusUpdated(int row, const QString& status);
    void finished();

private:
    QList<LoginAccount> m_accounts;
    QString m_gamePath;
    bool m_running;
    
    bool processAccount(const LoginAccount& account, int index);
};

class LoginScript : public QObject {
    Q_OBJECT
    
public:
    explicit LoginScript(QObject* parent = nullptr);
    ~LoginScript();

    void start(const QList<LoginAccount>& accounts, const QString& gamePath);
    void stop();
    bool isRunning() const { return m_isRunning; }

signals:
    void started();
    void stopped();
    void messageUpdated(const QString& message);
    void statusUpdated(int row, const QString& status);

private:
    QThread* m_thread;
    LoginWorker* m_worker;
    bool m_isRunning;
}; 