#include "Script/MemoryScript.h"
#include <QDebug>
#include "Util/WindowHelper.h"
#include "main.h"

MemoryWorker::MemoryWorker(QObject* parent)
    : QObject(parent)
    , m_running(false)
    , m_miningEnabled(false)
    , m_miningInterval(1000)
    , m_miningIntervalEnabled(false)
    , m_breathingEnabled(false)
    , m_reviveEnabled(false)
    , m_wallhackEnabled(false)
    , m_teleportEnabled(false) {
}

void MemoryWorker::setParams(const QList<HWND>& windows) {
    m_windows = windows;
    m_running = true;
}

void MemoryWorker::stop() {
    m_running = false;
}

void MemoryWorker::setMining(bool enabled, int interval) {
    m_miningEnabled = enabled;
    m_miningInterval = interval;
}

void MemoryWorker::setBreathing(bool enabled) {
    m_breathingEnabled = enabled;
}

void MemoryWorker::setRevive(bool enabled) {
    m_reviveEnabled = enabled;
}

void MemoryWorker::setWallhack(bool enabled) {
    m_wallhackEnabled = enabled;
}

void MemoryWorker::setTeleport(bool enabled) {
    m_teleportEnabled = enabled;
}

void MemoryWorker::setMiningInterval(bool enabled) {
    m_miningIntervalEnabled = enabled;
}

void MemoryWorker::processWindow(HWND hwnd) {
    if (GameWindow* window = findGameWindowByHwnd(hwnd)) {
        HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, window->pid);
        if (!handle) {
            emit messageUpdated(QString("无法访问进程[%1]").arg((quintptr)hwnd));
            return;
        }
        
        try {
            // 更新窗口状态
            window->task = "内存功能执行中";
            
            // 执行内存功能
            if (m_miningEnabled) {
                // 实现秒矿功能
            }
            
            if (m_breathingEnabled) {
                // 实现水下呼吸功能
            }
            
            // ... 其他功能实现 ...
            
            CloseHandle(handle);
            
        } catch (const std::exception& e) {
            emit messageUpdated(QString("内存操作失败：%1").arg(e.what()));
            CloseHandle(handle);
        }
    }
}

void MemoryWorker::doWork() {
    if (!DM || m_windows.isEmpty()) {
        emit finished();
        return;
    }
    
    try {
        while (m_running) {
            for (HWND hwnd : m_windows) {
                if (!m_running) break;
                
                if (!IsWindow(hwnd)) {
                    emit messageUpdated(QString("句柄[%1]已失效").arg((quintptr)hwnd));
                    continue;
                }
                
                processWindow(hwnd);
            }
            
            Sleep(100);  // 避免CPU占用过高
        }
    } catch (const std::exception& e) {
        emit messageUpdated(QString("内存功能执行失败：%1").arg(e.what()));
    }
    
    // 恢复窗口状态
    for (HWND hwnd : m_windows) {
        if (GameWindow* window = findGameWindowByHwnd(hwnd)) {
            window->task = "等待中";
        }
    }
    
    emit finished();
}

MemoryScript::MemoryScript(QObject* parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_worker(nullptr)
    , m_isRunning(false) {
}

MemoryScript::~MemoryScript() {
    if (m_isRunning) {
        stop();
    }
}

void MemoryScript::start(const QList<HWND>& windows) {
    if (m_isRunning) return;
    
    if (!DM) {
        emit messageUpdated("错误：大漠插件未初始化！");
        return;
    }
    
    if (windows.isEmpty()) {
        emit messageUpdated("错误：没有选中任何窗口！");
        return;
    }
    
    // 创建工作线程
    m_thread = new QThread(this);
    m_worker = new MemoryWorker();
    m_worker->moveToThread(m_thread);
    
    // 连接信号
    connect(m_thread, &QThread::started, m_worker, &MemoryWorker::doWork);
    connect(m_worker, &MemoryWorker::finished, m_thread, &QThread::quit);
    connect(m_worker, &MemoryWorker::finished, m_worker, &MemoryWorker::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    connect(m_worker, &MemoryWorker::messageUpdated, this, &MemoryScript::messageUpdated);
    
    // 设置参数并启动线程
    m_worker->setParams(windows);
    m_thread->start();
    
    m_isRunning = true;
    emit started();
}

void MemoryScript::stop() {
    if (!m_isRunning) return;
    
    if (m_worker) {
        m_worker->stop();
    }
    
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
    }
    
    m_thread = nullptr;
    m_worker = nullptr;
    m_isRunning = false;
    
    emit stopped();
}

void MemoryScript::setMining(bool enabled, int interval) {
    if (m_worker) {
        m_worker->setMining(enabled, interval);
    }
}

void MemoryScript::setBreathing(bool enabled) {
    if (m_worker) {
        m_worker->setBreathing(enabled);
    }
}

void MemoryScript::setRevive(bool enabled) {
    if (m_worker) {
        m_worker->setRevive(enabled);
    }
}

void MemoryScript::setWallhack(bool enabled) {
    if (m_worker) {
        m_worker->setWallhack(enabled);
    }
}

void MemoryScript::setTeleport(bool enabled) {
    if (m_worker) {
        m_worker->setTeleport(enabled);
    }
}

void MemoryScript::setMiningInterval(bool enabled) {
    if (m_worker) {
        m_worker->setMiningInterval(enabled);
    }
} 