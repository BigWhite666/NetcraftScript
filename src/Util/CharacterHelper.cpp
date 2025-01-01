#include "Util/CharacterHelper.h"
#include "MemoryRead/Memory.h"
#include "MemoryRead/GameOffsets.h"
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
        Vector3 current = getPosition(hwnd);
        
        // 计算角度
        float dx = target.x - current.x;
        float dy = target.y - current.y;
        float dz = (target.z - 1) - current.z;
        
        float distance = std::sqrt(dx*dx + dy*dy);
        float angleX = -(180/M_PI) * std::atan2(dx, dy);
        float angleY = -(180/M_PI) * std::atan2(dz, distance);

        // 获取进程信息
        DWORD pid = GetPid(hwnd);
        int msvcr120 = ListProcessModules(pid, "MSVCR120.dll");
        if (msvcr120 != 0) {
            HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (handle) {
                DWORD_PTR angleBase = msvcr120 + 0x000DFE1C;
                bool result = writeAngle(handle, angleBase, angleX, angleY);
                CloseHandle(handle);
                return result;
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
        DWORD pid = GetPid(hwnd);
        int msvcr120 = ListProcessModules(pid, "MSVCR120.dll");
        if (msvcr120 != 0) {
            HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (handle) {
                DWORD_PTR angleBase = msvcr120 + 0x000DFE1C;
                bool result = writeAngle(handle, angleBase, angleX, angleY);
                CloseHandle(handle);
                return result;
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
    DWORD pid = GetPid(hwnd);
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    
    if (handle) {
        GameOffsets::Initialize(hwnd);
        pos.x = ReadMemory<float>(handle, GameOffsets::Position::X);
        pos.y = ReadMemory<float>(handle, GameOffsets::Position::Y);
        pos.z = ReadMemory<float>(handle, GameOffsets::Position::Z);
        CloseHandle(handle);
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

bool CharacterHelper::writeAngle(HANDLE handle, DWORD_PTR angleBase, float angleX, float angleY) {
    try {
        WriteMemory<float>(handle, CalculateAddress(handle, angleBase, {0x19C}), angleX);
        WriteMemory<float>(handle, CalculateAddress(handle, angleBase, {0x1A0}), angleY);
        return true;
    } catch (const std::exception& e) {
        qDebug() << "写入角度失败：" << e.what();
        return false;
    }
} 