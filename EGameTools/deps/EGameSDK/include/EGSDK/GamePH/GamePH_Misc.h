#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <EGSDK\Exports.h>

namespace EGSDK::GamePH {
	extern EGameSDK_API const DWORD GetCurrentGameVersion();
	extern EGameSDK_API const std::string GameVerToStr(DWORD version);
	extern EGameSDK_API const std::string GetCurrentGameVersionStr();
	extern EGameSDK_API void ShowTPPModel(bool showTPPModel);
	extern EGameSDK_API bool ReloadJumps();
}
