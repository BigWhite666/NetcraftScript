#include "Util/WindowHelper.h"
#include "main.h"
#include "MemoryRead/Memory.h"
#include "Util/GameWindow.h"
#include <QDebug>

bool WindowHelper::checkDM() {
    if (!DM) {
        qDebug() << "大漠插件未初始化";
        return false;
    }
    return true;
}

bool WindowHelper::bindWindow(HWND hwnd, QString& errorMsg, bool pressF1) {
    if (!checkDM()) {
        errorMsg = "大漠插件未初始化";
        return false;
    }

    try {
        // 绑定窗口
        long bindResult = DM->BindWindowEx(
            reinterpret_cast<long>(hwnd),
            "gdi",
            "dx",
            "dx.keypad.input.lock.api",
            "",
            0
        );

        if (bindResult != 1) {
            errorMsg = QString("句柄[%1]绑定失败").arg((quintptr)hwnd);
            return false;
        }

        // 绑定成功后自动居中鼠标
        // 获取窗口大小
        VARIANT height, width;
        DM->GetClientSize((long)hwnd, &width, &height);
        
        // 计算窗口中心点
        int centerX = width.intVal / 2;
        int centerY = height.intVal / 2;
        // 如果需要，按F1
        if (pressF1) {
            // 移动到窗口中心
            DM->MoveTo(centerX, centerY);
            DM->KeyPress(112);  // F1键
            Sleep(100);
        }

        return true;
    } catch (const std::exception& e) {
        errorMsg = QString("绑定窗口失败：%1").arg(e.what());
        return false;
    }
}

void WindowHelper::unbindWindow() {
    if (checkDM()) {
        DM->KeyPress(112);  // F1键
        DM->UnBindWindow();
    }
}

void WindowHelper::centerWindow(HWND hwnd) {
    if (!checkDM()) return;

    try {
        // 获取窗口大小
        VARIANT height, width;
        DM->GetClientSize((long)hwnd, &width, &height);
        
        // 计算窗口中心点
        int centerX = width.intVal / 2;
        int centerY = height.intVal / 2;
        
        // 移动到窗口中心
        DM->MoveTo(centerX, centerY);
    } catch (const std::exception& e) {
        qDebug() << "窗口居中失败：" << e.what();
    }
}

void WindowHelper::pressF1AndCenter(HWND hwnd) {
    if (!checkDM()) return;

    try {
        DM->KeyPress(112);  // F1键
        Sleep(100);
        centerWindow(hwnd);
    } catch (const std::exception& e) {
        qDebug() << "按F1并居中失败：" << e.what();
    }
}

struct WindowInfo {
    HWND hwnd;
    DWORD createTime;
};

BOOL CALLBACK WindowHelper::enumLatestWindowProc(HWND hwnd, LPARAM lParam) {
    wchar_t className[256];
    GetClassNameW(hwnd, className, sizeof(className)/sizeof(wchar_t));
    
    // 检查是否是游戏窗口
    if (wcscmp(className, L"CIrrDeviceWin32") == 0) {  // 使用宽字符字符串
        DWORD processId;
        GetWindowThreadProcessId(hwnd, &processId);
        
        // 获取进程创建时间
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
        if (hProcess) {
            FILETIME createTime, exitTime, kernelTime, userTime;
            if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
                WindowInfo* info = (WindowInfo*)lParam;
                ULARGE_INTEGER createTimeInt;
                createTimeInt.LowPart = createTime.dwLowDateTime;
                createTimeInt.HighPart = createTime.dwHighDateTime;
                
                // 更新最新的窗口
                if (info->hwnd == NULL || createTimeInt.QuadPart > info->createTime) {
                    info->hwnd = hwnd;
                    info->createTime = createTimeInt.QuadPart;
                }
            }
            CloseHandle(hProcess);
        }
    }
    return TRUE;
}

HWND WindowHelper::findLatestGameWindow() {
    WindowInfo info = { NULL, 0 };
    EnumWindows(enumLatestWindowProc, (LPARAM)&info);
    return info.hwnd;
}

QString WindowHelper::getCharacterName(HWND hwnd) {
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    
    if (!handle) {
        return "无法访问进程";
    }

    QString result;
    char buffer[18] = {0};
    SIZE_T bytesRead = 0;
    
    // 创建临时 GameWindow 来获取地址
    GameWindow window;
    window.hwnd = hwnd;
    window.pid = pid;
    window.initializeAddresses();
    
    if (ReadProcessMemory(handle, (LPVOID)window.addresses.characterName,
                         buffer, sizeof(buffer), &bytesRead)) {
        result = QString::fromLocal8Bit(buffer);
        if (result.isEmpty()) {
            result = "未登录";
        }
    } else {
        result = "未登录";
    }
    
    CloseHandle(handle);
    return result;
} 