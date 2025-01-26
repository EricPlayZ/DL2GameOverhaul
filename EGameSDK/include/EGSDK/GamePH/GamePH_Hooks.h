#pragma once
#include <stdint.h>
#include <EGSDK\Utils\Hook.h>

namespace EGSDK::GamePH {
	class GameDI_PH2;

	namespace Hooks {
		extern EGameSDK_API Utils::Hook::MHook<void*, uint64_t(*)(uint64_t), uint64_t> CreatePlayerHealthModuleHook;
		extern EGameSDK_API Utils::Hook::MHook<void*, uint64_t(*)(uint64_t), uint64_t> CreatePlayerInfeIctionModuleHook;

		extern EGameSDK_API bool didOnPostUpdateHookExecute;
		extern EGameSDK_API Utils::Hook::VTHook<GameDI_PH2*, void(*)(void*), void*> OnPostUpdateHook;
	}
}