#include "Script/LoginScript.h"
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QProcess>
#include <QDir>
#include "Util/WindowHelper.h"
#include "Util/InputHelper.h"
#include "MemoryRead/GameOffsets.h"
#include "main.h"

LoginWorker::LoginWorker(QObject* parent)
    : QObject(parent)
    , m_running(false) {
}

void LoginWorker::setParams(const QList<LoginAccount>& accounts, const QString& gamePath) {
    m_accounts = accounts;
    m_gamePath = gamePath;
    m_running = true;
}

void LoginWorker::stop() {
    if (!m_running) return;
    
    m_running = false;
    emit messageUpdated("正在停止...");
}

bool LoginWorker::processAccount(const LoginAccount& account, int index) {
    try {
        emit messageUpdated(QString("正在处理账号: %1").arg(account.account));
        emit statusUpdated(index, "处理中");
        
        // 启动游戏
        QString updatePath = QDir(m_gamePath).absoluteFilePath("updater.exe");
        emit messageUpdated(QString("更新程序路径: %1").arg(updatePath));
        
        if (!QFile::exists(updatePath)) {
            emit messageUpdated("找不到更新程序！");
            emit statusUpdated(index, "失败");
            return false;
        }
        
        // 使用 ShellExecute 启动程序
        HINSTANCE result = ShellExecuteA(
            NULL, "open", updatePath.toLocal8Bit().constData(),
            NULL, m_gamePath.toLocal8Bit().constData(), SW_SHOWNORMAL
        );

        if ((INT_PTR)result <= 32) {
            emit messageUpdated(QString("启动失败，错误码：%1").arg((INT_PTR)result));
            emit statusUpdated(index, "失败");
            return false;
        }

        // 等待游戏窗口出现
        emit messageUpdated("等待游戏窗口...");
        HWND gameHwnd = nullptr;
        int retryCount = 0;
        
        while (!gameHwnd && retryCount < 30 && m_running) {
            gameHwnd = WindowHelper::findLatestGameWindow();
            if (!gameHwnd) {
                Sleep(3000);
                retryCount++;
            }
        }
        
        if (!gameHwnd) {
            emit messageUpdated("等待游戏窗口超时！");
            emit statusUpdated(index, "失败");
            return false;
        }

        // 绑定游戏窗口
        emit messageUpdated("正在绑定游戏窗口...");
        QString errorMsg;
        if (!WindowHelper::bindWindow(gameHwnd, errorMsg, false)) {
            emit messageUpdated(QString("绑定游戏窗口失败：%1").arg(errorMsg));
            emit statusUpdated(index, "失败");
            return false;
        }
        emit messageUpdated("游戏窗口绑定成功");

        // 等待登录窗口出现
        emit messageUpdated("等待登录窗口...");
        HWND loginWnd = nullptr;
        HWND ieServerWnd = nullptr;
        
        while (m_running) {
            // 点击界面触发登录窗口
            InputHelper::moveToAndClick(1500, 200);
            Sleep(1000);
            
            // 查找登录对话框窗口(#32770)
            EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
                auto params = (std::pair<HWND, HWND*>*)lParam;
                HWND gameHwnd = params->first;
                HWND* ieServerWnd = params->second;
                
                if (GetParent(hwnd) == gameHwnd) {
                    char className[256];
                    GetClassNameA(hwnd, className, sizeof(className));
                    
                    if (strcmp(className, "#32770") == 0) {
                        // 找到登录对话框后，查找其中的 Internet Explorer_Server
                        EnumChildWindows(hwnd, [](HWND childHwnd, LPARAM lParam) -> BOOL {
                            char childClassName[256];
                            GetClassNameA(childHwnd, childClassName, sizeof(childClassName));
                            
                            if (strcmp(childClassName, "Internet Explorer_Server") == 0) {
                                *(HWND*)lParam = childHwnd;
                                return FALSE;  // 停止枚举
                            }
                            return TRUE;
                        }, (LPARAM)ieServerWnd);
                        
                        return FALSE;  // 找到了，停止枚举
                    }
                }
                return TRUE;
            }, (LPARAM)&std::make_pair(gameHwnd, &ieServerWnd));

            if (ieServerWnd) {
                emit messageUpdated("找到登录窗口");
                break;
            }
            
            emit messageUpdated("尝试触发登录窗口...");
        }

        if (!m_running) {
            WindowHelper::unbindWindow();
            emit statusUpdated(index, "已中断");
            return false;
        }

        // 解绑游戏窗口，绑定登录窗口
        WindowHelper::unbindWindow();
        emit messageUpdated("正在绑定登录窗口...");
        
        if (!WindowHelper::bindWindow(ieServerWnd, errorMsg, false)) {
            emit messageUpdated(QString("绑定登录窗口失败：%1").arg(errorMsg));
            emit statusUpdated(index, "失败");
            return false;
        }
        emit messageUpdated("登录窗口绑定成功");

        // 确保窗口激活
        SetForegroundWindow(gameHwnd);
        Sleep(500);

        // 输入账号密码
        InputHelper::moveToAndClick(377, 209);  // 账号输入框
        Sleep(500);
        // 清空输入框 - 使用退格键清空
        for(int i = 0; i < 30; i++) {  // 假设最多30个字符
            InputHelper::keyPress(VK_BACK);  // 退格键
            Sleep(10);
        }
        InputHelper::inputString(account.account);
        Sleep(500);
        
        InputHelper::moveToAndClick(377, 260);  // 密码输入框
        Sleep(500);
        // 清空输入框
        for(int i = 0; i < 30; i++) {
            InputHelper::keyPress(VK_BACK);
            Sleep(10);
        }
        InputHelper::inputString(account.password);
        Sleep(500);
        
        InputHelper::moveToAndClick(200, 300);  // 登录按钮

        Sleep(500);
        InputHelper::moveToAndClick(340, 400);  // 登录按钮

        // 解绑登录窗口
        WindowHelper::unbindWindow();
        
        // 等待指定间隔
        int remainingTime = account.interval;
        while (remainingTime > 0 && m_running) {
            Sleep(1000);
            remainingTime--;
        }
        
        emit statusUpdated(index, "完成");
        return true;
    } catch (const std::exception& e) {
        WindowHelper::unbindWindow();  // 确保出错时也解绑窗口
        emit messageUpdated(QString("处理失败：%1").arg(e.what()));
        emit statusUpdated(index, "失败");
        return false;
    }
}

void LoginWorker::doWork() {
    if (!DM) {
        emit messageUpdated("错误：大漠插件未初始化！");
        emit finished();
        return;
    }
    
    try {
        for (int i = 0; i < m_accounts.size() && m_running; i++) {
            const auto& account = m_accounts[i];
            
            if (!m_running) {
                emit messageUpdated("处理被中断");
                break;
            }
            
            // 处理当前账号
            if (!processAccount(account, i)) {
                if (!m_running) {
                    break;
                }
                // 如果登录失败，继续下一个账号
                continue;
            }
        }
    } catch (const std::exception& e) {
        emit messageUpdated(QString("执行失败：%1").arg(e.what()));
    }
    
    emit messageUpdated(m_running ? "任务结束" : "已停止");
    emit finished();
}

LoginScript::LoginScript(QObject* parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_worker(nullptr)
    , m_isRunning(false) {
}

LoginScript::~LoginScript() {
    if (m_isRunning) {
        stop();
    }
    
    // 确保线程和工作对象被正确清理
    if (m_thread) {
        if (m_thread->isRunning()) {
            m_thread->quit();
            m_thread->wait();
        }
        delete m_thread;
    }
    
    if (m_worker) {
        delete m_worker;
    }
}

void LoginScript::start(const QList<LoginAccount>& accounts, const QString& gamePath) {
    if (m_isRunning) return;
    
    if (!DM) {
        emit messageUpdated("错误：大漠插件未初始化！");
        return;
    }
    
    if (accounts.isEmpty()) {
        emit messageUpdated("错误：没有账号信息！");
        return;
    }
    
    // 创建工作线程
    m_thread = new QThread();
    m_worker = new LoginWorker();
    m_worker->moveToThread(m_thread);
    
    // 连接信号
    connect(m_thread, &QThread::started, m_worker, &LoginWorker::doWork);
    connect(m_worker, &LoginWorker::finished, this, [this]() {
        m_isRunning = false;
        m_thread->quit();
    });
    
    connect(m_worker, &LoginWorker::messageUpdated, this, &LoginScript::messageUpdated);
    connect(m_worker, &LoginWorker::statusUpdated, this, &LoginScript::statusUpdated);
    
    // 设置参数并启动线程
    m_worker->setParams(accounts, gamePath);
    m_thread->start();
    
    m_isRunning = true;
    emit started();
}

void LoginScript::stop() {
    if (!m_isRunning) return;
    
    m_isRunning = false;
    
    if (m_worker) {
        // 停止工作线程
        m_worker->stop();
        
        // 等待线程完成
        if (m_thread) {
            // 断开所有连接
            m_thread->disconnect();
            m_worker->disconnect();
            
            // 退出线程
            m_thread->quit();
            if (!m_thread->wait(1000)) {
                qDebug() << "线程无法正常退出，强制终止";
                m_thread->terminate();
                m_thread->wait();
            }
        }
    }
    
    emit stopped();
} 