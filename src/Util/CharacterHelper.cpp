#include "Util/CharacterHelper.h"
#include "MemoryRead/Memory.h"
#include "Util/GameWindow.h"
#include "main.h"
#include <cmath>
#include <QDebug>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool CharacterHelper::checkDM() {
    if (!DM) {
        qDebug() << "大漠插件未初始化";
        return false;
    }
    return true;
}

bool CharacterHelper::moveForward(bool start) {
    if (!checkDM()) return false;
    return start ? DM->KeyDown('W') == 1 : DM->KeyUp('W') == 1;
}

bool CharacterHelper::jump(bool start) {
    if (!checkDM()) return false;
    return start ? DM->KeyDown(VK_SPACE) == 1 : DM->KeyUp(VK_SPACE) == 1;
}

bool CharacterHelper::crouch(bool start) {
    if (!checkDM()) return false;
    return start ? DM->KeyDown('C') == 1 : DM->KeyUp('C') == 1;
}

bool CharacterHelper::aimTarget(HWND hwnd, const Vector3& target) {
    try {
        if (GameWindow* window = GameWindows::findByHwnd(hwnd)) {
            Vector3 current = getPosition(hwnd);

            // 计算角度
            float dx = target.x - current.x;
            float dy = target.y - current.y;
            float dz = (target.z - 1) - current.z;

            float distance = std::sqrt(dx*dx + dy*dy);
            float angleX = -(180/M_PI) * std::atan2(dx, dy);
            float angleY = -(180/M_PI) * std::atan2(dz, distance);
            
            HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, window->pid);
            if (handle) {
                WriteMemory<float>(handle, window->addresses.angleX, angleX);
                WriteMemory<float>(handle, window->addresses.angleY, angleY);
                CloseHandle(handle);
                return true;
            }
        }
        return false;
    } catch (const std::exception& e) {
        qDebug() << "瞄准失败：" << e.what();
        return false;
    }
}

bool CharacterHelper::setAngle(HWND hwnd, float angleX, float angleY) {
    try {
        if (GameWindow* window = GameWindows::findByHwnd(hwnd)) {
            HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, window->pid);
            if (handle) {
                WriteMemory<float>(handle, window->addresses.angleX, angleX);
                WriteMemory<float>(handle, window->addresses.angleY, angleY);
                CloseHandle(handle);
                return true;
            }
        }
        return false;
    } catch (const std::exception& e) {
        qDebug() << "设置角度失败：" << e.what();
        return false;
    }
}

Vector3 CharacterHelper::getPosition(HWND hwnd) {
    Vector3 pos = {0, 0, 0};
    if (GameWindow* window = GameWindows::findByHwnd(hwnd)) {
        HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, window->pid);
        if (handle) {
            pos.x = ReadMemory<float>(handle, window->addresses.positionX);
            pos.y = ReadMemory<float>(handle, window->addresses.positionY);
            pos.z = ReadMemory<float>(handle, window->addresses.positionZ);
            CloseHandle(handle);
        }
    }
    return pos;
}

float CharacterHelper::getDistance(const Vector3& pos1, const Vector3& pos2) {
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    float dz = pos2.z - pos1.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

bool CharacterHelper::isMoving() {
    if (!checkDM()) return false;
    return DM->GetKeyState('W') == 1;
}

bool CharacterHelper::isJumping() {
    if (!checkDM()) return false;
    return DM->GetKeyState(VK_SPACE) == 1;
}

bool CharacterHelper::isCrouching() {
    if (!checkDM()) return false;
    return DM->GetKeyState('C') == 1;
}

bool CharacterHelper::isFlying() {
    if (!checkDM()) return false;
    
    // 检查是否处于飞行状态
    // 这里可能需要根据具体游戏机制来判断
    // 暂时通过检查是否同时按住空格或C键来判断
    return false;
}

bool CharacterHelper::doubleJump() {
    if (!checkDM()) return false;
    
    // 快速双击空格
    DM->KeyPress(VK_SPACE);
    Sleep(50);
    DM->KeyPress(VK_SPACE);
    Sleep(100);  // 等待一下确保进入飞行状态
    
    return true;
}

bool CharacterHelper::enterFlyMode() {
    if (!checkDM()) return false;
    
    if (!isFlying()) {
        // 尝试进入飞行模式
        return doubleJump();
    }
    return true;  // 已经在飞行状态
}

bool CharacterHelper::flyUp(bool start) {
    if (!checkDM()) return false;
    return start ? DM->KeyDown(VK_SPACE) == 1 : DM->KeyUp(VK_SPACE) == 1;
}

bool CharacterHelper::flyDown(bool start) {
    if (!checkDM()) return false;
    return start ? DM->KeyDown('C') == 1 : DM->KeyUp('C') == 1;
}
