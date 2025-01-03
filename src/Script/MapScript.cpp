//
// Created by Administrator on 24-12-23.
//

#include "Script/MapScript.h"
#include "Util/WindowHelper.h"
#include "main.h"

MapScript::MapScript(QObject* parent) 
    : QObject(parent)
    , m_isRunning(false) {
}

MapScript::~MapScript() {
    stop();
}

void MapScript::run(const GameWindow& window, const Vector3& target) {
    if (!m_isRunning) {
        m_isRunning = true;
        
        QString errorMsg;
        if (!WindowHelper::bindWindow(window.hwnd, errorMsg,true)) {
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
                    break;  // 到达目标位置，退出循环
                }

                Sleep(50);
            }
        } catch (const std::exception& e) {
            emit messageUpdated(QString("跑图失败：%1").arg(e.what()));
        }

        // 确保按键释放
        CharacterHelper::moveForward(false);
        CharacterHelper::jump(false);
        CharacterHelper::crouch(false);
        WindowHelper::unbindWindow();
        
        m_isRunning = false;
    }
}

void MapScript::stop() {
    m_isRunning = false;
}

