#include "Script/DebugScript.h"
#include <QDebug>
#include "Util/WindowHelper.h"
#include "main.h"
#include "MemoryRead/Memory.h"
#include "dm/dmutils.h"
#include "Util/GameWindow.h"

DebugWorker::DebugWorker(QObject* parent)
    : QObject(parent)
    , m_running(false) {
}

void DebugWorker::setParams(const QList<HWND>& windows) {
    m_windows = windows;
    m_running = true;
}

void DebugWorker::stop() {
    m_running = false;
}

bool DebugWorker::processWindow(HWND hwnd) {
    try {
        QString errorMsg;
        if (!WindowHelper::bindWindow(hwnd, errorMsg, false)) {
            emit messageUpdated(errorMsg);
            return false;
        }
        
        emit messageUpdated(QString("句柄[%1]绑定成功").arg((quintptr)hwnd));
        
        // 获取窗口大小
        VARIANT height, width;
        DM->GetClientSize((long)hwnd, &width, &height);
        int centerX = width.intVal / 2;
        int centerY = height.intVal / 2;
        
        // 移动到窗口中心并点击
        DM->MoveTo(centerX, centerY);
        Sleep(100);
        DM->KeyPress(112);  // F1
        Sleep(1000);
        
        // 获取角色信息并显示
        GameWindow window;
        window.hwnd = hwnd;
        window.pid = GetWindowThreadProcessId(hwnd, NULL);
        window.initializeAddresses();
        
        HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, window.pid);
        if (handle) {
            char buffer[18] = {0};
            SIZE_T bytesRead = 0;
            
            if (ReadProcessMemory(handle, (LPVOID)window.addresses.characterName,
                                buffer, sizeof(buffer), &bytesRead)) {
                QString name = QString::fromLocal8Bit(buffer);
                emit messageUpdated(QString("角色名称：%1").arg(name));
            }
            
            CloseHandle(handle);
        }
        
        // 解绑窗口
        WindowHelper::unbindWindow();
        return true;
        
    } catch (const std::exception& e) {
        emit messageUpdated(QString("处理窗口[%1]失败：%2").arg((quintptr)hwnd).arg(e.what()));
        WindowHelper::unbindWindow();
        return false;
    }
}

void DebugWorker::doWork() {
    if (!DM || m_windows.isEmpty()) {
        emit finished();
        return;
    }
    
    try {
        for (HWND hwnd : m_windows) {
            if (!m_running) break;
            
            if (!IsWindow(hwnd)) {
                emit messageUpdated(QString("句柄[%1]已失效").arg((quintptr)hwnd));
                continue;
            }
            
            processWindow(hwnd);
        }
    } catch (const std::exception& e) {
        emit messageUpdated(QString("调试执行失败：%1").arg(e.what()));
    }
    
    emit finished();
}

DebugScript::DebugScript(QObject* parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_worker(nullptr)
    , m_isRunning(false) {
}

DebugScript::~DebugScript() {
    if (m_isRunning) {
        stop();
    }
}

void DebugScript::start(const QList<HWND>& windows) {
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
    m_worker = new DebugWorker();
    m_worker->moveToThread(m_thread);
    
    // 连接信号
    connect(m_thread, &QThread::started, m_worker, &DebugWorker::doWork);
    connect(m_worker, &DebugWorker::finished, m_thread, &QThread::quit);
    connect(m_worker, &DebugWorker::finished, m_worker, &DebugWorker::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    connect(m_worker, &DebugWorker::messageUpdated, this, &DebugScript::messageUpdated);
    
    // 设置参数并启动线程
    m_worker->setParams(windows);
    m_thread->start();
    
    m_isRunning = true;
    emit started();
}

void DebugScript::stop() {
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