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
            if (strcmp(className, "D3D Window") == 0) {
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
        window.hotkey = "";  // 默认无快捷键
        window.task = "等待中";
        window.isChecked = false;
        window.position = CharacterHelper::getPosition(hwnd);
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