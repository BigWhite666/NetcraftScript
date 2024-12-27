//
// Created by Administrator on 24-12-18.
//

#ifndef MEMORY_H
#define MEMORY_H

#pragma once
#include <iostream>
#include <vector>
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

// 获取主线程地址
uint64_t GetThreadstackoAddress(int pid);
// 读取模块地址
int ListProcessModules(int gamePid, const char* modulesName);
//  读地址
int CalculateAddress(HANDLE handle, int threadstack, std::vector<int> offsets);
// 加强版读取内存
void ReadMemoryEx(HANDLE handle, int address, void* retval, size_t size);
// HWND转PID
int GetPid(HWND hwnd);

// 把字符串转UTF-8
std::string GetUtf8String(std::string utf8String);
// 读取内存
#ifndef READMEMORY_H
#define READMEMORY_H

// 读内存
template<typename T> T ReadMemory(HANDLE hProcess, long address)
{
    T value;
    ReadProcessMemory(hProcess, (LPVOID)address, &value, sizeof(value), nullptr);
    return value;
}

// 写内存
template<typename T> void WriteMemory(HANDLE hProcess, long address, T value)
{
    WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(value), nullptr);
}

#endif

#endif //MEMORY_H
