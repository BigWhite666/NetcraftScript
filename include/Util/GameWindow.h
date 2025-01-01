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
    
    GameWindow() : hwnd(nullptr), pid(0), isChecked(false), position{0,0,0} {}
};

// 游戏窗口管理类
class GameWindows {
public:
    static QList<GameWindow> windows;  // 存储所有游戏窗口
    static void refresh();             // 刷新窗口列表
    static void clear();               // 清空窗口列表
    static GameWindow* findByHwnd(HWND hwnd);  // 通过句柄查找窗口
}; 