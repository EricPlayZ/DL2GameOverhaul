#pragma once
#include <atomic>
#include <EGSDK\Utils\Values.h>

#ifdef _DEBUG
#ifndef LLMH_IMPL_DISABLE_DEBUG
#define LLMH_IMPL_DISABLE_DEBUG  // this is for disabling low-level mouse hook in case ure trying to debug and u dont want ur pc to die lol
#endif
#endif

namespace EGT {
    constexpr const char* MOD_VERSION_STR = "v1.2.4";
    constexpr unsigned long MOD_VERSION = 10204;
    constexpr unsigned long GAME_VER_COMPAT = 12001;

    namespace Core {
        extern std::atomic<bool> exiting;
    }
}