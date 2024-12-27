//
// Created by Administrator on 24-12-18.
//
#include "MemoryRead/Memory.h"

#include <algorithm>


// 从进程内存中读取指定地址的整数值
int raddr(HANDLE handle, int addr) {
    unsigned int ret = 0;
    ReadProcessMemory(handle, (LPCVOID)addr, &ret, sizeof(int), nullptr);
    return ret;
}

// 计算多级指针地址
int CalculateAddress(HANDLE handle, int threadstack, std::vector<int> offsets) {
    int addr = threadstack;
    for (int offset : offsets) {
        addr = raddr(handle, addr) + offset;
    }
    return addr;
}

// 通过窗口句柄获取进程ID
int GetPid(HWND hwnd){
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}

// 从进程内存中读取数据的通用函数
void ReadMemoryEx(HANDLE handle, int address, void* retval, size_t size) {
    ReadProcessMemory(handle, (LPCVOID)address, retval, size, nullptr);
}

// 获取指定进程中指定模块的基址
int ListProcessModules(int gamePid, const char* modulesName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, gamePid);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("读取模块地址错误: CreateToolhelp32Snapshot failed. Error: %d\n", GetLastError());
        return -1;
    }
    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);
    if (!Module32First(hSnapshot, &me32)) {
        CloseHandle(hSnapshot);
        printf("Module32First failed. Error: %d\n", GetLastError());
        return -1;
    }
    do {
        if (WideCharToMultiByte(CP_ACP, 0, me32.szModule, -1, NULL, 0, NULL, NULL) > 0) {
            char szModule[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, me32.szModule, -1, szModule, MAX_PATH, NULL, NULL);
            if (strcmp(modulesName, szModule) == 0) {
                return reinterpret_cast<unsigned long long int>(me32.modBaseAddr);
            }
        }
    } while (Module32Next(hSnapshot, &me32));
    CloseHandle(hSnapshot);
    return -1;
}

// 将UTF-8字符串转换为GBK编码
std::string GetUtf8String(std::string utf8String) {
    // 将UTF-8编码的字符串转换为宽字符
    int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, nullptr, 0);
    wchar_t* wideStr = new wchar_t[wideCharLen];
    MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, wideStr, wideCharLen);

    // 将宽字符转换为GBK编码
    int gbkLen = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
    char* gbkStr = new char[gbkLen];
    WideCharToMultiByte(CP_ACP, 0, wideStr, -1, gbkStr, gbkLen, nullptr, nullptr);

    std::string gbkString(gbkStr);
    delete[] wideStr;
    delete[] gbkStr;
    return gbkString;
}

#if _WIN64
typedef uint64_t StackPtrType;
#else
typedef uint32_t StackPtrType;
#endif

constexpr int PTR_SIZE = sizeof(StackPtrType);
constexpr int STACK_SIZE = 1024 * PTR_SIZE;

struct CLIENT_ID
{
    PVOID UniqueProcess;
    PVOID UniqueThread;
};

struct THREAD_BASIC_INFORMATION
{
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    DWORD Priority;
    DWORD BasePriority;
};

enum THREADINFOCLASS
{
    ThreadBasicInformation,
};

// 获取线程栈顶地址
uint64_t find_thread_stack_top(HANDLE process_handle, HANDLE thread_handle)
{
    bool loadedManually = false;
    HMODULE module = GetModuleHandle(TEXT("ntdll.dll"));
    if (!module)
    {
        module = LoadLibrary(TEXT("ntdll.dll"));
        if (!module)
            return 0;

        loadedManually = true;
    }

    uint64_t result = 0;

    NTSTATUS(__stdcall * NtQueryInformationThread)(HANDLE ThreadHandle, THREADINFOCLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength);
    NtQueryInformationThread = reinterpret_cast<decltype(NtQueryInformationThread)>(GetProcAddress(module, "NtQueryInformationThread"));
    if (NtQueryInformationThread)
    {
        NT_TIB tib = { 0 };
        THREAD_BASIC_INFORMATION tbi = { 0 };

        NTSTATUS status = NtQueryInformationThread(thread_handle, ThreadBasicInformation, &tbi, sizeof(tbi), nullptr);
        if (status >= 0) {
            ReadProcessMemory(process_handle, tbi.TebBaseAddress, &tib, sizeof(tbi), nullptr);
            result = (uint64_t)tib.StackBase;
        }
    }

    if (loadedManually)
        FreeLibrary(module);
    return result;
}

// 获取指定线程的栈地址
uint64_t find_threadstack_for_thread(HANDLE process_handle, DWORD thread_id)
{
    HANDLE thread_handle = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, thread_id);
    if (!thread_handle) {
        return 0;
    }
    uint64_t stacktop = find_thread_stack_top(process_handle, thread_handle);
    CloseHandle(thread_handle);

    if (!stacktop) {
        return 0;
    }

    MODULEINFO mi;
    HMODULE kernel_module = GetModuleHandle(TEXT("kernel32.dll"));
    if (!kernel_module) {
        return 0;
    }
    GetModuleInformation(process_handle, kernel_module, &mi, sizeof(mi));
    StackPtrType buf[1024]{};
    if (ReadProcessMemory(process_handle, (LPCVOID)(stacktop - STACK_SIZE), buf, STACK_SIZE, NULL)) {
        for (int i = STACK_SIZE / PTR_SIZE - 1; i >= 0; --i) {
            if (buf[i] >= (uint64_t)mi.lpBaseOfDll && buf[i] <= (uint64_t)mi.lpBaseOfDll + mi.SizeOfImage) {
                return stacktop - STACK_SIZE + (i * PTR_SIZE);
                break;
            }
        }
    }
    return 0;
}

// 获取进程的第一个线程ID
uint64_t get_first_thread_id(DWORD pid)
{
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (h == INVALID_HANDLE_VALUE)
        return -1;

    THREADENTRY32 te{};
    te.dwSize = sizeof(te);
    if (Thread32First(h, &te))
    {
        do {
            if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
            {
                if (te.th32OwnerProcessID == pid)
                    return te.th32ThreadID;
            }
            te.dwSize = sizeof(te);
        } while (Thread32Next(h, &te));
    }

    return -1;
}

// 获取主线程栈地址
uint64_t GetThreadstackoAddress(int pid)
{
    try
    {
        HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!process_handle || process_handle == INVALID_HANDLE_VALUE) {
            return 0;
        }
        DWORD thread_id = get_first_thread_id(pid);
        uint64_t threadstack_0 = find_threadstack_for_thread(process_handle, thread_id);
        // std::cout << threadstack_0 << std::endl;
        CloseHandle(process_handle);
        return threadstack_0;
    }
    catch (std::invalid_argument&)
    {
        return 0;
    }
}

// 获取模块大小
int GetModuleSize(HANDLE hProcess, DWORD baseAddress) {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(hProcess, reinterpret_cast<LPCVOID>(baseAddress), &mbi, sizeof(mbi))) {
        return mbi.RegionSize;
    }
    return 0;
}

// 获取模块结束地址
int GetModuleEndAddress(int gamePid, const char* moduleName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, gamePid);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cout << "Error: CreateToolhelp32Snapshot failed. Error: " << GetLastError() << std::endl;
        return 0;
    }

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);
    if (!Module32First(hSnapshot, &me32)) {
        CloseHandle(hSnapshot);
        std::cout << "Error: Module32First failed. Error: " << GetLastError() << std::endl;
        return 0;
    }

    do {
        char szModule[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, me32.szModule, -1, szModule, MAX_PATH, NULL, NULL);
        if (strcmp(moduleName, szModule) == 0) {
            DWORD moduleSize = GetModuleSize(GetCurrentProcess(), reinterpret_cast<DWORD>(me32.modBaseAddr));
            if (moduleSize > 0) {
                return reinterpret_cast<unsigned long long int>(me32.modBaseAddr) + moduleSize - 1;
            }
        }
    } while (Module32Next(hSnapshot, &me32));

    CloseHandle(hSnapshot);
    return 0;
}


