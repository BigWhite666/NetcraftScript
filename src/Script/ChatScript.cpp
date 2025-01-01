#include "Script/ChatScript.h"
#include <QDebug>
#include "Util/WindowHelper.h"
#include "Util/GameWindow.h"
#include "MemoryRead/Memory.h"
#include "main.h"

ChatWorker::ChatWorker(QObject* parent)
    : QObject(parent)
    , m_running(false) {
}

void ChatWorker::setParams(const QString& content, int interval, const QList<HWND>& windows) {
    m_content = content;
    m_interval = interval;
    m_windows = windows;
    m_running = true;
}

void ChatWorker::stop() {
    m_running = false;
}

void ChatWorker::doWork() {
    if (!DM || m_windows.isEmpty()) {
        emit finished();
        return;
    }
    
    try {
        // 遍历所有窗口进行绑定
        for (HWND hwnd : m_windows) {
            if (!m_running) break;  // 检查是否需要停止
            
            if (!IsWindow(hwnd)) {
                emit messageUpdated(QString("句柄[%1]已失效").arg((quintptr)hwnd));
                continue;
            }
            
            // 绑定窗口
            long bindResult = DM->BindWindowEx(
                reinterpret_cast<long>(hwnd),
                "gdi",
                "dx",
                "dx.keypad.input.lock.api",
                "",
                0
            );

            if (bindResult != 1) {
                emit messageUpdated(QString("句柄[%1]绑定失败").arg((quintptr)hwnd));
                continue;
            }
            
            emit messageUpdated(QString("句柄[%1]绑定成功，开始喊话").arg((quintptr)hwnd));
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

            // 开始喊话循环
            while (m_running) {
                // 发送回车
                DM->KeyPress(13);
                Sleep(100);
                
                // 发送喊话内容
                std::wstring content = m_content.toStdWString();
                for (const wchar_t& ch : content) {
                    if (ch >= L'0' && ch <= L'9') {
                        // 数字使用 KeyPressStr
                        wchar_t numStr[2] = {ch, L'\0'};
                        DM->KeyPressStr(numStr,10);
                    } else {
                        // 非数字使用 SendString
                        wchar_t charStr[2] = {ch, L'\0'};
                        DM->SendString((long)hwnd, charStr);
                    }
                    Sleep(10);  // 每个字符之间添加短暂延迟
                }

                Sleep(100);
                DM->KeyPress(13);  // 发送消息
                Sleep(100);

                emit messageUpdated(QString("句柄[%1]发送喊话：%2").arg((quintptr)hwnd).arg(m_content));
                
                // 等待指定间隔
                for (int i = 0; i < m_interval && m_running; ++i) {
                    Sleep(1000);
                }
            }
            
            // 解绑窗口
            DM->UnBindWindow();
        }
    } catch (const std::exception& e) {
        emit messageUpdated(QString("喊话发送失败：%1").arg(e.what()));
    }
    
    emit finished();
}

ChatScript::ChatScript(QObject* parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_worker(nullptr)
    , m_isRunning(false) {
}

ChatScript::~ChatScript() {
    stop();
}

void ChatScript::start(const QString& content, int interval, const QList<HWND>& windows) {
    if (m_isRunning) return;
    
    if (!DM) {
        emit messageUpdated("错误：大漠插件未初始化！");
        return;
    }
    
    // 获取选中的窗口
    QList<HWND> selectedWindows;
    for (const auto& window : GameWindows::windows) {
        if (window.isChecked) {
            selectedWindows.append(window.hwnd);
        }
    }
    
    if (selectedWindows.isEmpty()) {
        emit messageUpdated("错误：没有选中任何窗口！");
        return;
    }
    
    // 创建工作线程
    m_thread = new QThread(this);
    m_worker = new ChatWorker();
    m_worker->moveToThread(m_thread);
    
    // 连接信号
    connect(m_thread, &QThread::started, m_worker, &ChatWorker::doWork);
    connect(m_worker, &ChatWorker::finished, m_thread, &QThread::quit);
    connect(m_worker, &ChatWorker::finished, m_worker, &ChatWorker::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    connect(m_worker, &ChatWorker::messageUpdated, this, &ChatScript::messageUpdated);
    
    // 设置参数并启动线程
    m_worker->setParams(content, interval, selectedWindows);
    m_thread->start();
    
    m_isRunning = true;
    emit started();
}

void ChatScript::stop() {
    DM->KeyPress(27);
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
    emit messageUpdated("喊话已停止");
} 