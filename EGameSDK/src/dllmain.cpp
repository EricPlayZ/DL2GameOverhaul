#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <semaphore>
#include <EGSDK\Utils\Hook.h>

namespace EGSDK {
    namespace Core {
        extern std::counting_semaphore<4> maxHookThreads;

        extern void InitLogger();

        extern DWORD64 WINAPI MainThread(HMODULE hModule);
        extern void Cleanup();
    }
}

static HANDLE mainThreadHandle{};

BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD64 reasonForCall, void* lpReserved) {
    switch (reasonForCall) {
    case DLL_PROCESS_ATTACH: {
        EGSDK::Core::InitLogger();

        SPDLOG_INFO("DLL_PROCESS_ATTACH");
        MH_Initialize();

        SPDLOG_INFO("Disabling thread library calls");
        DisableThreadLibraryCalls(moduleHandle);

        SPDLOG_INFO("Creating main thread");
        mainThreadHandle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)EGSDK::Core::MainThread, moduleHandle, 0, nullptr);

        if (!mainThreadHandle) {
            SPDLOG_ERROR("Failed to create main thread");
            return FALSE;
        }
        break;
    }
    case DLL_PROCESS_DETACH:
        SPDLOG_INFO("DLL_PROCESS_DETACH");
        EGSDK::Core::Cleanup();

        if (mainThreadHandle) {
            SPDLOG_INFO("Closing main thread handle");
            CloseHandle(mainThreadHandle);
        }
        SPDLOG_INFO("Freeing library and exiting thread");
        FreeLibraryAndExitThread(moduleHandle, 0);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

