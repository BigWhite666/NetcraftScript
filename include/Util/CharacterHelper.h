#pragma once

#include <QObject>
#include <windows.h>
#include "dm/dmutils.h"

struct Vector3 {
    float x;
    float y;
    float z;
};

class CharacterHelper : public QObject {
    Q_OBJECT
    
public:
    // 角色移动相关
    static bool moveForward(bool start = true);  // 开始/停止前进
    static bool jump(bool start = true);         // 开始/停止跳跃
    static bool crouch(bool start = true);       // 开始/停止下蹲
    
    // 角色朝向相关
    static bool aimTarget(HWND hwnd, const Vector3& target);  // 瞄准目标位置
    static bool setAngle(HWND hwnd, float angleX, float angleY);  // 设置视角角度
    
    // 角色位置相关
    static Vector3 getPosition(HWND hwnd);  // 获取角色位置
    static float getDistance(const Vector3& pos1, const Vector3& pos2);  // 计算两点距离
    
    // 角色状态相关
    static bool isMoving();     // 是否在移动
    static bool isJumping();    // 是否在跳跃
    static bool isCrouching();  // 是否在下蹲
    
    // 飞行相关方法
    static bool isFlying();                  // 检查是否在飞行状态
    static bool enterFlyMode();              // 进入飞行模式
    static bool flyUp(bool start = true);    // 开始/停止向上飞行
    static bool flyDown(bool start = true);  // 开始/停止向下飞行

private:
    static bool checkDM();
    static bool doubleJump();  // 快速双击空格
}; 