#include "Util/GameWindow.h"
#include "Util/WindowHelper.h"
#include "MemoryRead/Memory.h"
#include "main.h"

QList<GameWindow> GameWindows::windows;

void GameWindows::refresh() {
    clear();
    
    // 枚举所有窗口
    std::vector<HWND> gameHwnds;
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto hwnds = reinterpret_cast<std::vector<HWND>*>(lParam);
        if (IsWindow(hwnd) && IsWindowVisible(hwnd)) {
            char className[256] = {0};
            GetClassNameA(hwnd, className, sizeof(className));
            if (strcmp(className, "CIrrDeviceWin32") == 0) {
                hwnds->push_back(hwnd);
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&gameHwnds));
    
    // 添加到窗口列表
    for (HWND hwnd : gameHwnds) {
        GameWindow window;
        window.hwnd = hwnd;
        window.pid = GetPid(hwnd);
        window.role = WindowHelper::getCharacterName(hwnd);
        window.hotkey = "";
        window.task = "等待中";
        window.isChecked = false;
        window.position = CharacterHelper::getPosition(hwnd);
        window.initializeAddresses();  // 初始化内存地址
        windows.append(window);
    }
}

void GameWindows::clear() {
    windows.clear();
}

GameWindow* GameWindows::findByHwnd(HWND hwnd) {
    for (auto& window : windows) {
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
    
    // 计算实际地址
    addresses.characterName = CalculateAddress(handle, threadstack - 0x00000B6C, {0x10, 0x1F0});
    
    addresses.positionX = CalculateAddress(handle, threadstack - 0x00000B6C, {0x10, 0x448});
    addresses.positionY = addresses.positionX + 0x4;
    addresses.positionZ = addresses.positionX + 0x8;
    
    addresses.moveX = CalculateAddress(handle, threadstack - 0x00000B6C, {0x10, 0x244});
    addresses.moveY = addresses.moveX + 0x4;
    addresses.moveZ = addresses.moveX + 0x8;
    
    addresses.angleX = CalculateAddress(handle, msvcr120 + 0x000DFE1C, {0x19C});
    addresses.angleY = addresses.angleX + 0x4;
    
    CloseHandle(handle);
} 