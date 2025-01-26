#pragma once
#include <stdint.h>
#include <vector>
#include <memory>
#include <semaphore>
#include <spdlog\spdlog.h>
#include <EGSDK\Exports.h>

#ifdef _DEBUG
#ifndef EXCP_HANDLER_DISABLE_DEBUG
#define EXCP_HANDLER_DISABLE_DEBUG  // this is for disabling exception handling in case ure trying to debug and u dont want ur debugger to keep crying about errors lol
#endif
#endif

namespace EGSDK {
    static constexpr uint32_t GAME_VER_COMPAT = 12001;
    static constexpr std::array<uint32_t, 2> SUPPORTED_GAME_VERSIONS = { 11200, 12001 };

    namespace Core {
        extern EGameSDK_API std::vector<spdlog::sink_ptr> spdlogSinks;
#ifdef EGameSDK_EXPORTS
        extern std::counting_semaphore<4> maxHookThreads;
#endif

        extern EGameSDK_API int rendererAPI;
        extern EGameSDK_API uint32_t gameVer;

        extern EGameSDK_API void SetDefaultLoggerSettings(std::shared_ptr<spdlog::logger> logger);
#ifdef EGameSDK_EXPORTS
        extern void InitLogger();
#endif

        extern EGameSDK_API std::string GetSDKStoragePath();

#ifdef EGameSDK_EXPORTS
        extern void OnPostUpdate();

        extern DWORD64 WINAPI MainThread(HMODULE hModule);
        extern void Cleanup();
#endif
    }
}