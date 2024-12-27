//
// Created by Administrator on 24-12-23.
//

#include "Script/MapScript.h"
#include <QDebug>
#include "MemoryRead/Memory.h"
#include "MemoryRead/GameOffsets.h"
#include "main.h"
#include <cmath>
#include "Util/WindowHelper.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MapWorker::MapWorker(QObject* parent)
    : QObject(parent)
    , m_running(false) {
}

void MapWorker::setParams(const QString& mapName, int loopCount, const QList<HWND>& windows) {
    m_mapName = mapName;
    m_loopCount = loopCount;
    m_windows = windows;
    m_running = true;
}

void MapWorker::stop() {
    m_running = false;
}

void MapWorker::AimTarget(HWND hwnd, float targetX, float targetY, float targetZ) {
    try {
        // 获取进程ID和句柄
        DWORD pid = GetPid(hwnd);
        HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!handle) {
            emit messageUpdated("无法访问进程");
            return;
        }

        // 获取玩家当前位置
        GameOffsets::Initialize(hwnd);
        float playerX = ReadMemory<float>(handle, GameOffsets::Position::X);
        float playerY = ReadMemory<float>(handle, GameOffsets::Position::Y);
        float playerZ = ReadMemory<float>(handle, GameOffsets::Position::Z);

        // 计算角度
        float disX = targetX - playerX;
        float disY = (targetZ - 1) - playerZ;
        float disZ = targetY - playerY;
        float dis = std::sqrt(disX * disX + disZ * disZ);
        float radToDeg = 180 / M_PI;
        float setAngleX = -(radToDeg * std::atan2(disX, disZ));
        float setAngleY = -(radToDeg * std::atan2(disY, dis));

        // 写入角度 - 修改这部分
        int msvcr120 = ListProcessModules(pid, "MSVCR120.dll");
        if (msvcr120 != 0) {
            DWORD_PTR angleBase = msvcr120 + 0x000DFE1C;
            WriteMemory<float>(handle, CalculateAddress(handle, angleBase, {0x19C}), setAngleX);
            WriteMemory<float>(handle, CalculateAddress(handle, angleBase, {0x1A0}), setAngleY);
        }

        CloseHandle(handle);

        // 确保按下W键
        if (m_running) {
            DM->KeyDown('W');  // 按住前进键
            emit messageUpdated("开始移动");
        }

    } catch (const std::exception& e) {
        emit messageUpdated(QString("自瞄失败：%1").arg(e.what()));
    }
}

void MapWorker::doWork() {
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
            
            // 使用通用的窗口绑定工具
            QString errorMsg;
            if (!WindowHelper::bindWindow(hwnd, errorMsg, false)) {
                emit messageUpdated(errorMsg);
                continue;
            }
            
            emit messageUpdated(QString("句柄[%1]绑定成功").arg((quintptr)hwnd));
            WindowHelper::pressF1AndCenter(hwnd);
            
            // 开始跑图循环
            bool isForwardKeyDown = false;
            
            while (m_running) {
                // 目标坐标
                float targetX = 54880.0f;
                float targetY = 9330.0f;
                float targetZ = 15.0f;

                // 获取当前位置
                DWORD pid = GetPid(hwnd);
                HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
                if (!handle) continue;

                GameOffsets::Initialize(hwnd);
                float currentX = ReadMemory<float>(handle, GameOffsets::Position::X);
                float currentY = ReadMemory<float>(handle, GameOffsets::Position::Y);
                float currentZ = ReadMemory<float>(handle, GameOffsets::Position::Z);

                // 计算与目标点的距离
                float dx = currentX - targetX;
                float dy = currentY - targetY;
                float dz = currentZ - targetZ;
                float distance = std::sqrt(dx*dx + dy*dy + dz*dz);

                emit messageUpdated(QString("距离目标点: %1").arg(distance));  // 添加距离提示

                // 根据距离控制移动
                if (distance > 2.0f) {
                    // 瞄准目标点并移动
                    AimTarget(hwnd, targetX, targetY, targetZ);
                    if (!isForwardKeyDown) {
                        DM->KeyDown('W');  // 按住前进键
                        isForwardKeyDown = true;
                        emit messageUpdated("开始移动");
                    }
                } else {
                    if (isForwardKeyDown) {
                        DM->KeyUp('W');    // 释放前进键
                        isForwardKeyDown = false;
                        emit messageUpdated("到达目标点");
                        break;  // 到达目标点，退出循环
                    }
                }

                // 高度控制
                float heightDiff = targetZ - currentZ;
                if (heightDiff > 5.0f && isForwardKeyDown) {
                    DM->KeyDown(VK_SPACE);  // 按住空格键上升
                    DM->KeyUp('C');         // 确保下降键释放
                } else {
                    DM->KeyDown('C');       // 按住C键下降
                    DM->KeyUp(VK_SPACE);    // 确保上升键释放
                }

                Sleep(50);  // 短暂延迟，避免CPU占用过高
                CloseHandle(handle);
            }

            // 确保所有按键释放
            DM->KeyUp('W');
            DM->KeyUp(VK_SPACE);
            DM->KeyUp('C');
            
            // 按ESC和Y退出
            DM->KeyPress(VK_ESCAPE);
            Sleep(100);
            
            // 解绑窗口
            WindowHelper::unbindWindow();
        }
    } catch (const std::exception& e) {
        emit messageUpdated(QString("跑图执行失败：%1").arg(e.what()));
    }
    
    emit finished();
}

MapScript::MapScript(QObject* parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_worker(nullptr)
    , m_isRunning(false) {
}

MapScript::~MapScript() {
    if (m_isRunning) {
        stop();
    }
}

void MapScript::start(const QString& mapName, int loopCount, const QList<HWND>& windows) {
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
    m_worker = new MapWorker();
    m_worker->moveToThread(m_thread);
    
    // 连接信号
    connect(m_thread, &QThread::started, m_worker, &MapWorker::doWork);
    connect(m_worker, &MapWorker::finished, m_thread, &QThread::quit);
    connect(m_worker, &MapWorker::finished, m_worker, &MapWorker::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    connect(m_worker, &MapWorker::messageUpdated, this, &MapScript::messageUpdated);
    connect(m_worker, &MapWorker::progressUpdated, this, &MapScript::progressUpdated);
    
    // 设置参数并启动线程
    m_worker->setParams(mapName, loopCount, windows);
    m_thread->start();
    
    m_isRunning = true;
    emit started();
}

void MapScript::stop() {
    DM->KeyPress(27);
    if (!m_isRunning) return;
    
    if (m_worker) {
        m_worker->stop();
    }
    
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
    }
    
    // 清理资源
    m_thread = nullptr;
    m_worker = nullptr;
    m_isRunning = false;
    
    emit stopped();
}
