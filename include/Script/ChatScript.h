#pragma once
#include <QObject>
#include <QThread>
#include <windows.h>
#include <QList>
#include <QString>
#include "dm/dmutils.h"

// 创建喊话工作线程类
class ChatWorker : public QObject {
    Q_OBJECT
    
public:
    explicit ChatWorker(QObject* parent = nullptr);
    
    // 修改参数顺序，与 ChatScript::start 保持一致
    void setParams(const QList<HWND>& windows, const QString& content, int interval);
    void stop();
    
signals:
    void finished();
    void messageUpdated(const QString& msg);
    
public slots:
    void doWork();
    
private:
    QString m_content;
    int m_interval;
    QList<HWND> m_windows;
    bool m_running;
};

class ChatScript : public QObject {
    Q_OBJECT
    
public:
    explicit ChatScript(QObject* parent = nullptr);
    ~ChatScript();
    
    void start(const QList<HWND>& windows, const QString& content, int interval);
    void stop();
    
signals:
    void started();
    void stopped();
    void messageUpdated(const QString& msg);
    
private:
    QThread* m_thread;
    ChatWorker* m_worker;
    bool m_isRunning;
}; 