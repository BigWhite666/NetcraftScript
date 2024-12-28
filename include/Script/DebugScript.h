#pragma once

#include <QObject>
#include <QThread>
#include <QList>
#include <windows.h>
#include "dm/dmutils.h"

// 工作线程类
class DebugWorker : public QObject {
    Q_OBJECT
    
public:
    explicit DebugWorker(QObject* parent = nullptr);
    void setParams(const QList<HWND>& windows);
    void stop();

public slots:
    void doWork();  // 执行调试任务

signals:
    void messageUpdated(const QString& message);
    void finished();

private:
    QList<HWND> m_windows;
    bool m_running;
    
    bool processWindow(HWND hwnd);  // 处理单个窗口的调试任务
};

class DebugScript : public QObject {
    Q_OBJECT
    
public:
    explicit DebugScript(QObject* parent = nullptr);
    ~DebugScript();

    void start(const QList<HWND>& windows);
    void stop();
    bool isRunning() const { return m_isRunning; }

signals:
    void started();
    void stopped();
    void messageUpdated(const QString& message);

private:
    QThread* m_thread;
    DebugWorker* m_worker;
    bool m_isRunning;
}; 