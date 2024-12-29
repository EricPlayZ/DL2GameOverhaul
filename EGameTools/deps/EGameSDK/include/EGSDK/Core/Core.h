#pragma once
#include <atomic>
#include <vector>
#include <memory>
#include <spdlog\sinks\rotating_file_sink.h>
#include <EGSDK\Exports.h>

#ifdef _DEBUG
#ifndef EXCP_HANDLER_DISABLE_DEBUG
#define EXCP_HANDLER_DISABLE_DEBUG  // this is for disabling exception handling in case ure trying to debug and u dont want ur debugger to keep crying about errors lol
#endif
#endif

namespace EGSDK {
    constexpr unsigned long GAME_VER_COMPAT = 12001;

    namespace Core {
        extern EGameSDK_API std::atomic<bool> exiting;
        extern EGameSDK_API std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> spdlogSink;

        extern EGameSDK_API int rendererAPI;
        extern EGameSDK_API unsigned long gameVer;
    }
}