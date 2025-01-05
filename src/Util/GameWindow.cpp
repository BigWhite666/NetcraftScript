#include "Util/GameWindow.h"
#include "Util/WindowHelper.h"
#include "MemoryRead/Memory.h"
#include "main.h"

// 定义全局变量
std::vector<GameWindow> g_gameWindows;

void refreshGameWindows() {
    g_gameWindows.clear();
    
    // 枚举所有窗口
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        if (IsWindow(hwnd) && IsWindowVisible(hwnd)) {
            char className[256] = {0};
            GetClassNameA(hwnd, className, sizeof(className));
            if (strcmp(className, "CIrrDeviceWin32") == 0) {
                GameWindow window;
                window.hwnd = hwnd;
                window.pid = GetPid(hwnd);
                window.role = WindowHelper::getCharacterName(hwnd);
                window.position = CharacterHelper::getPosition(hwnd);
                window.initializeAddresses();
                g_gameWindows.push_back(window);
            }
        }
        return TRUE;
    }, 0);
}

GameWindow* findGameWindowByHwnd(HWND hwnd) {
    for (auto& window : g_gameWindows) {
        if (window.hwnd == hwnd) {
            return &window;
        }
    }
    return nullptr;
}

void GameWindow::initializeAddresses() {
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!handle) return;
    
    // 获取基址
    int threadstack = GetThreadstackoAddress(pid);
    int msvcr120 = ListProcessModules(pid, "MSVCR120.dll");
    int netcraft = ListProcessModules(pid, "netcraft.exe");
    int openal32 = ListProcessModules(pid, "OpenAL32.dll");

    // 计算实际地址
    addresses.characterName = CalculateAddress(handle, threadstack - 0x00000B6C, {0x10, 0x1F0});
    
    addresses.positionX = CalculateAddress(handle, threadstack - 0x00000B6C, {0x10, 0x448});
    addresses.positionY = addresses.positionX + 0x8;
    addresses.positionZ = addresses.positionX + 0x4;
    
    addresses.moveX = CalculateAddress(handle, threadstack - 0x00000B6C, {0x10, 0x244});
    addresses.moveY = addresses.moveX + 0x4;
    addresses.moveZ = addresses.moveX + 0x8;
    
    addresses.angleX = CalculateAddress(handle, msvcr120 + 0x000DFE1C, {0x19C});
    addresses.angleY = addresses.angleX + 0x4;
    
    CloseHandle(handle);
} 