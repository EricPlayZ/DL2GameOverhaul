#include <EGSDK\Core\Core.h>
#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\GamePH_Hooks.h>
#include <EGSDK\GamePH\GameDI_PH2.h>
#include <EGSDK\GamePH\PlayerHealthModule.h>
#include <EGSDK\GamePH\PlayerInfectionModule.h>
#include <EGSDK\Utils\Hook.h>

namespace EGSDK::GamePH {
	namespace Hooks {
#pragma region CreatePlayerHealthModule
		Utils::Hook::MHook<void*, uint64_t(*)(uint64_t), uint64_t> CreatePlayerHealthModuleHook{ "CreatePlayerHealthModule", &OffsetManager::Get_CreatePlayerHealthModule, [](uint64_t playerHealthModule) -> uint64_t {
			PlayerHealthModule::EmplaceBack(reinterpret_cast<PlayerHealthModule*>(playerHealthModule));

			return CreatePlayerHealthModuleHook.ExecuteCallbacksWithOriginal(playerHealthModule);
		} };
#pragma endregion

#pragma region CreatePlayerInfectionModule
		Utils::Hook::MHook<void*, uint64_t(*)(uint64_t), uint64_t> CreatePlayerInfectionModuleHook{ "CreatePlayerInfectionModule", &OffsetManager::Get_CreatePlayerInfectionModule, [](uint64_t playerInfectionModule) -> uint64_t {
			PlayerInfectionModule::EmplaceBack(reinterpret_cast<PlayerInfectionModule*>(playerInfectionModule));

			return CreatePlayerInfectionModuleHook.ExecuteCallbacksWithOriginal(playerInfectionModule);
		} };
#pragma endregion

#pragma region OnPostUpdate
		bool didOnPostUpdateHookExecute = false;

		Utils::Hook::VTHook<GameDI_PH2*, void(*)(void*), void*> OnPostUpdateHook{ "OnPostUpdate", &GameDI_PH2::Get, [](void* pGameDI_PH2) -> void {
			didOnPostUpdateHookExecute = true;
			Core::OnPostUpdate();

			return OnPostUpdateHook.ExecuteCallbacksWithOriginal(pGameDI_PH2);
		} };
#pragma endregion
	}
}