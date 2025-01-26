#pragma once
#include <stdint.h>
#include <string>
#include <EGSDK\Exports.h>

namespace EGSDK::GamePH {
	extern EGameSDK_API const uint32_t GetCurrentGameVersion();
	extern EGameSDK_API const std::string GameVerToStr(uint32_t version);
	extern EGameSDK_API const std::string GetCurrentGameVersionStr();
	extern EGameSDK_API void ShowTPPModel(bool showTPPModel);
	extern EGameSDK_API bool ReloadJumps();
}
