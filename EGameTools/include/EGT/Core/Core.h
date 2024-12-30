#pragma once
#include <atomic>
#include <EGSDK\Utils\Values.h>

namespace EGT {
    constexpr const char* MOD_VERSION_STR = "v1.2.4";
    constexpr unsigned long MOD_VERSION = 10204;
    constexpr unsigned long GAME_VER_COMPAT = 12001;

    namespace Core {
        extern std::atomic<bool> exiting;
    }
}