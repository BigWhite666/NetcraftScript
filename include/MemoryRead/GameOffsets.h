//
// Created by Administrator on 24-12-19.
//

#ifndef GAMEOFFSETS_H
#define GAMEOFFSETS_H
#pragma once
#include <windows.h>
#include "Memory.h"
#include <vector>

// 游戏内存偏移定义
class GameOffsets {
public:
    // 初始化函数，计算所有实际地址
    static void Initialize(HWND hwnd) {
        // 获取进程ID和句柄
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

        // 获取基址
        int threadstack = GetThreadstackoAddress(pid);
        int msvcr120 = ListProcessModules(pid, "MSVCR120.dll");
        int netcraft = ListProcessModules(pid, "netcraft.exe");
        int openal32 = ListProcessModules(pid, "OpenAL32.dll");

        // 计算实际地址
        Character::NAME = CalculateAddress(handle, threadstack - 0x00000B6C, {0x10, 0x1F0});

        Position::X = CalculateAddress(handle, threadstack - 0x00000B6C, {0x10, 0x448});
        Position::Z = Position::X + 0x4;
        Position::Y = Position::X + 0x8;

        Position::MoveX = CalculateAddress(handle, threadstack - 0x00000B6C, {0x10, 0x244});
        Position::MoveZ = Position::MoveX + 0x4;
        Position::MoveY = Position::MoveX + 0x8;

        Position::AngleX = CalculateAddress(handle, msvcr120 + 0x000DFE1C, {0x19C});
        Position::AngleY = Position::AngleX + 0x4;

        CloseHandle(handle);
    }

    // 角色相关地址
    struct Character {
        static inline DWORD_PTR NAME = 0;  // 角色名称地址
        static inline DWORD_PTR HP = 0;    // 血量地址
        static inline DWORD_PTR MP = 0;    // 魔法值地址
        // ... 其他属性地址
    };

    // 坐标相关地址
    struct Position {
        static inline DWORD_PTR X = 0;  // X坐标地址
        static inline DWORD_PTR Y = 0;  // Y坐标地址
        static inline DWORD_PTR Z = 0;  // Z坐标地址
        static inline DWORD_PTR MoveX = 0;  // X坐标地址
        static inline DWORD_PTR MoveY = 0;  // Y坐标地址
        static inline DWORD_PTR MoveZ = 0;  // Z坐标地址
        static inline DWORD_PTR AngleX = 0;  // 视角X地址
        static inline DWORD_PTR AngleY = 0;  // 视角Y地址


    };

    // 状态相关地址
    struct Status {
        static inline DWORD_PTR IS_FLYING = 0;      // 飞行状态地址
        static inline DWORD_PTR IS_UNDERGROUND = 0;  // 地下状态地址
    };
};
#endif //GAMEOFFSETS_H
