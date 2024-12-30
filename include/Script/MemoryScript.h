#pragma once

#include <QObject>
#include <QThread>
#include <QList>
#include <windows.h>
#include "dm/dmutils.h"

// 工作线程类
class MemoryWorker : public QObject {
    Q_OBJECT
    
public:
    explicit MemoryWorker(QObject* parent = nullptr);
    void setParams(const QList<HWND>& windows);
    void stop();

    // 内存功能接口
    void setMining(bool enabled, int interval);
    void setBreathing(bool enabled);
    void setRevive(bool enabled);
    void setWallhack(bool enabled);
    void setTeleport(bool enabled);
    void setMiningInterval(bool enabled);

public slots:
    void doWork();  // 执行内存功能

signals:
    void messageUpdated(const QString& message);
    void finished();

private:
    QList<HWND> m_windows;
    bool m_running;
    
    // 功能状态
    bool m_miningEnabled;
    int m_miningInterval;
    bool m_breathingEnabled;
    bool m_reviveEnabled;
    bool m_wallhackEnabled;
    bool m_teleportEnabled;
    bool m_miningIntervalEnabled;
    
    // 处理单个窗口的内存功能
    void processWindow(HWND hwnd);
};

class MemoryScript : public QObject {
    Q_OBJECT
    
public:
    explicit MemoryScript(QObject* parent = nullptr);
    ~MemoryScript();

    void start(const QList<HWND>& windows);
    void stop();
    bool isRunning() const { return m_isRunning; }

    // 内存功能控制接口
    void setMining(bool enabled, int interval);
    void setBreathing(bool enabled);
    void setRevive(bool enabled);
    void setWallhack(bool enabled);
    void setTeleport(bool enabled);
    void setMiningInterval(bool enabled);

signals:
    void started();
    void stopped();
    void messageUpdated(const QString& message);

private:
    QThread* m_thread;
    MemoryWorker* m_worker;
    bool m_isRunning;
}; 