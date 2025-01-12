#include <Windows.h>
#include <semaphore>
#include <EGSDK\Utils\Hook.h>
#include <EGT\Core\Core.h>
#include <EGT\Engine\Engine_Hooks.h>

static HANDLE mainThreadHandle{};

BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD64 reasonForCall, void* lpReserved) {
    switch (reasonForCall) {
    case DLL_PROCESS_ATTACH: {
        EGT::Core::InitLogger();

        MH_Initialize();

        SPDLOG_INFO("Initializing hooks");
        EGT::Engine::Hooks::MountDataPaksHook.HookLoop();
        EGT::Engine::Hooks::AuthenticateDataAddNewFileHook.HookLoop();
        EGT::Engine::Hooks::FsCheckZipCrcHook.HookLoop();
        EGT::Engine::Hooks::FsOpenHook.HookLoop();

        SPDLOG_INFO("Disabling thread library calls");
        DisableThreadLibraryCalls(moduleHandle);

        SPDLOG_INFO("Creating main thread");
        mainThreadHandle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)EGT::Core::MainThread, moduleHandle, 0, nullptr);

        if (!mainThreadHandle) {
            SPDLOG_ERROR("Failed to create main thread");
            return FALSE;
        }
        break;
    }
    case DLL_PROCESS_DETACH: {
        SPDLOG_INFO("DLL_PROCESS_DETACH");
        EGT::Core::Cleanup();

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