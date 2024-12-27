#include <pch.h>

namespace Core {
    extern void EnableConsole();
    extern void DisableConsole();

    extern std::counting_semaphore<4> maxHookThreads;

    extern void InitLogger();

    extern DWORD64 WINAPI MainThread(HMODULE hModule);
    extern void Cleanup();
}

namespace Engine {
    namespace Hooks {
        extern Utils::Hook::MHook<LPVOID, DWORD64(*)(DWORD64, UINT, UINT, DWORD64*, DWORD64(*)(DWORD64, DWORD, DWORD64, char*, int), INT16, DWORD64, UINT)> MountDataPaksHook;
        extern Utils::Hook::MHook<LPVOID, LPVOID(*)(LPVOID)> AuthenticateDataAddNewFileHook;
        extern Utils::Hook::MHook<LPVOID, bool(*)(LPVOID)> FsCheckZipCrcHook;
        extern Utils::Hook::MHook<LPVOID, DWORD64(*)(DWORD64, DWORD, DWORD)> FsOpenHook;
    }
}

static HANDLE mainThreadHandle{};

BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD64 reasonForCall, LPVOID lpReserved) {
    switch (reasonForCall) {
    case DLL_PROCESS_ATTACH: {
        Core::EnableConsole();
        Core::InitLogger();

        SPDLOG_INFO("DLL_PROCESS_ATTACH");
        MH_Initialize();

        SPDLOG_INFO("Initializing hooks");
        Engine::Hooks::MountDataPaksHook.HookLoop();
        Engine::Hooks::AuthenticateDataAddNewFileHook.HookLoop();
        Engine::Hooks::FsCheckZipCrcHook.HookLoop();
        Engine::Hooks::FsOpenHook.HookLoop();

        SPDLOG_INFO("Disabling thread library calls");
        DisableThreadLibraryCalls(moduleHandle);

        SPDLOG_INFO("Creating main thread");
        mainThreadHandle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Core::MainThread, moduleHandle, 0, nullptr);

        if (!mainThreadHandle) {
            SPDLOG_ERROR("Failed to create main thread");
            return FALSE;
        }
        break;
    }
    case DLL_PROCESS_DETACH: {
        SPDLOG_INFO("DLL_PROCESS_DETACH");
        Core::Cleanup();
        Core::DisableConsole();

        if (mainThreadHandle) {
            SPDLOG_INFO("Closing main thread handle");
            CloseHandle(mainThreadHandle);
        }
        SPDLOG_INFO("Freeing library and exiting thread");
        FreeLibraryAndExitThread(moduleHandle, 0);
        break;
    }
    default:
        return FALSE;
    }
    return TRUE;
}