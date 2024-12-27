#pragma once
#include <QString>
#include <QObject>
#include <QThread>
#include <QList>
#include <windows.h>
#include "dm/dmutils.h"

// 创建喊话工作线程类
class ChatWorker : public QObject {
    Q_OBJECT
    
public:
    explicit ChatWorker(QObject* parent = nullptr);
    void setParams(const QString& content, int interval, const QList<HWND>& windows);
    void stop();

public slots:
    void doWork();  // 执行喊话任务

signals:
    void messageUpdated(const QString& message);
    void finished();

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

    void start(const QString& content, int interval, const QList<HWND>& windows);
    void stop();
    bool isRunning() const { return m_thread != nullptr; }

signals:
    void started();
    void stopped();
    void messageUpdated(const QString& message);

private:
    QThread* m_thread;
    ChatWorker* m_worker;
    bool m_isRunning;
}; 