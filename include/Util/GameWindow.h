#pragma once

#include <windows.h>
#include <QString>
#include <vector>
#include "Util/CharacterHelper.h"

struct GameWindow {
    HWND hwnd;           // 窗口句柄
    DWORD pid;           // 进程ID
    QString role;        // 角色名称
    QString hotkey;      // 快捷键
    QString task;        // 当前任务状态
    bool isChecked;      // 是否被选中
    Vector3 position;    // 当前位置
    
    struct MemoryAddresses {
        DWORD_PTR characterName;
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
    
    void initializeAddresses();  // 初始化内存地址
};

// 声明全局变量
extern std::vector<GameWindow> g_gameWindows;

// 辅助函数声明
void refreshGameWindows();  // 刷新窗口列表
GameWindow* findGameWindowByHwnd(HWND hwnd);  // 通过句柄查找窗口 