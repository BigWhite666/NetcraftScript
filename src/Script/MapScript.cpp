//
// Created by Administrator on 24-12-23.
//

#include "Script/MapScript.h"
#include <QDebug>
#include "MemoryRead/Memory.h"
#include "MemoryRead/GameOffsets.h"
#include "Util/WindowHelper.h"
#include "Util/GameWindow.h"
#include "main.h"
#include <cmath>
#include "Util/CharacterHelper.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MapScript::MapScript(QObject* parent)
    : QObject(parent)
    , m_isRunning(false) {
}

MapScript::~MapScript() {
    stop();
}

void MapScript::start(float targetX, float targetY, float targetZ) {
    if (m_isRunning) return;
    
    Vector3 target = {targetX, targetY, targetZ};
    
    // 获取选中的窗口
    QList<GameWindow> selectedWindows;
    for (const auto& window : GameWindows::windows) {
        if (window.isChecked && window.task == "等待中") {
            selectedWindows.append(window);
        }
    }
    
    if (selectedWindows.isEmpty()) {
        emit messageUpdated("错误：没有选中处于等待中状态的窗口！");
        return;
    }
    
    // 为每个窗口创建一个线程执行跑图任务
    for (const auto& window : selectedWindows) {
        QThread* thread = QThread::create([this, window, target]() {
            try {
                moveToPosition(window, target);
            } catch (const std::exception& e) {
                emit messageUpdated(QString("窗口[%1]跑图失败：%2")
                    .arg((quintptr)window.hwnd)
                    .arg(e.what()));
            }
        });
        
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        m_threads.append(thread);
        thread->start();
    }
    
    m_isRunning = true;
    emit started();
}

void MapScript::stop() {
    m_isRunning = false;
    
    // 等待所有线程完成
    for (auto thread : m_threads) {
        if (thread && thread->isRunning()) {
            thread->quit();
            thread->wait();
        }
    }
    
    m_threads.clear();
    emit stopped();
}

void MapScript::moveToPosition(const GameWindow& window, const Vector3& target) {
    QString errorMsg;
    if (!WindowHelper::bindWindow(window.hwnd, errorMsg)) {
        emit messageUpdated(errorMsg);
        return;
    }

    bool isForwardKeyDown = false;
    
    try {
        while (m_isRunning) {
            Vector3 currentPos = CharacterHelper::getPosition(window.hwnd);
            float distance = CharacterHelper::getDistance(currentPos, target);

            if (distance > 2.0f) {
                // 瞄准目标并移动
                CharacterHelper::aimTarget(window.hwnd, target);
                
                if (!isForwardKeyDown) {
                    CharacterHelper::moveForward(true);
                    isForwardKeyDown = true;
                }

                // 高度控制
                float heightDiff = target.z - currentPos.z;
                if (heightDiff > 5.0f) {
                    CharacterHelper::jump(true);
                    CharacterHelper::crouch(false);
                } else if (heightDiff < -5.0f) {
                    CharacterHelper::crouch(true);
                    CharacterHelper::jump(false);
                }
            } else {
                if (isForwardKeyDown) {
                    CharacterHelper::moveForward(false);
                    isForwardKeyDown = false;
                }
                CharacterHelper::jump(false);
                CharacterHelper::crouch(false);
                break;
            }

            Sleep(50);
        }
    } catch (const std::exception& e) {
        emit messageUpdated(QString("窗口[%1]跑图失败：%2")
            .arg((quintptr)window.hwnd)
            .arg(e.what()));
    }

    // 确保按键释放
    CharacterHelper::moveForward(false);
    CharacterHelper::jump(false);
    CharacterHelper::crouch(false);
    WindowHelper::unbindWindow();
}

Vector3 MapScript::getPlayerPosition(const GameWindow& window) {
    return CharacterHelper::getPosition(window.hwnd);
}

