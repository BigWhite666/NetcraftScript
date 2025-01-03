#pragma once

#include <windows.h>
#include <QString>
#include <QList>
#include "Util/CharacterHelper.h"  // 引入 Vector3 定义

struct GameWindow {
    HWND hwnd;           // 窗口句柄
    DWORD pid;           // 进程ID
    QString role;        // 角色名称
    QString hotkey;      // 快捷键
    QString task;        // 当前任务状态
    bool isChecked;      // 是否被选中
    Vector3 position;    // 当前位置
    
    // 添加内存地址
    struct MemoryAddresses {
        // 角色相关地址
        DWORD_PTR characterName;
        
        // 坐标相关地址
        DWORD_PTR positionX;
        DWORD_PTR positionY;
        DWORD_PTR positionZ;
        DWORD_PTR moveX;
        DWORD_PTR moveY;
        DWORD_PTR moveZ;
        DWORD_PTR angleX;
        DWORD_PTR angleY;
        
        MemoryAddresses() : characterName(0), 
            positionX(0), positionY(0), positionZ(0),
            moveX(0), moveY(0), moveZ(0),
            angleX(0), angleY(0) {}
    } addresses;
    
    GameWindow() : hwnd(nullptr), pid(0), isChecked(false), position{0,0,0} {}
    
    // 初始化内存地址
    void initializeAddresses();
};

// 游戏窗口管理类
class GameWindows {
public:
    static QList<GameWindow> windows;  // 存储所有游戏窗口
    static void refresh();             // 刷新窗口列表
    static void clear();               // 清空窗口列表
    static GameWindow* findByHwnd(HWND hwnd);  // 通过句柄查找窗口
}; 