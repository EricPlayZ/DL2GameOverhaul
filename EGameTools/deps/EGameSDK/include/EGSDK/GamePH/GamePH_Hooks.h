#pragma once
#include <EGSDK\Utils\Hook.h>

namespace EGSDK::GamePH {
	class GameDI_PH2;

	namespace Hooks {
		extern EGameSDK_API Utils::Hook::MHook<void*, DWORD64(*)(DWORD64), DWORD64> CreatePlayerHealthModuleHook;
		extern EGameSDK_API Utils::Hook::MHook<void*, DWORD64(*)(DWORD64), DWORD64> CreatePlayerInfeIctionModuleHook;
		extern EGameSDK_API Utils::Hook::VTHook<GameDI_PH2*, void(*)(void*), void*> OnPostUpdateHook;
	}
}