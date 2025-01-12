#pragma once
#include <Windows.h>
#include <atomic>
#include <semaphore>
#include <EGSDK\Utils\Values.h>

namespace EGT {
    constexpr const char* MOD_VERSION_STR = "v1.3.0";
    constexpr unsigned long MOD_VERSION = 10300;
    constexpr unsigned long GAME_VER_COMPAT = 12001;

    namespace Core {
        extern void OpenIOBuffer();
        extern void CloseIOBuffer();
        extern void EnableConsole();
        extern void DisableConsole();

        extern std::atomic<bool> exiting;
        extern std::counting_semaphore<4> maxHookThreads;

        extern void InitLogger();

        extern DWORD64 WINAPI MainThread(HMODULE hModule);
        extern void Cleanup();
    }
}