#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\GamePH_Hooks.h>
#include <EGSDK\GamePH\GameDI_PH2.h>
#include <EGSDK\GamePH\PlayerHealthModule.h>
#include <EGSDK\GamePH\PlayerInfectionModule.h>
#include <EGSDK\Utils\Hook.h>

namespace EGSDK::Core {
	extern void OnPostUpdate();
}

namespace EGSDK::GamePH {
	namespace Hooks {
#pragma region CreatePlayerHealthModule
		Utils::Hook::MHook<void*, DWORD64(*)(DWORD64), DWORD64> CreatePlayerHealthModuleHook{ "CreatePlayerHealthModule", &Offsets::Get_CreatePlayerHealthModule, [](DWORD64 playerHealthModule) {
			PlayerHealthModule::EmplaceBack(reinterpret_cast<PlayerHealthModule*>(playerHealthModule));

			CreatePlayerHealthModuleHook.ExecuteCallbacks(playerHealthModule);
			return CreatePlayerHealthModuleHook.pOriginal(playerHealthModule);
		}};
#pragma endregion

#pragma region CreatePlayerInfectionModule
		Utils::Hook::MHook<void*, DWORD64(*)(DWORD64), DWORD64> CreatePlayerInfectionModuleHook{ "CreatePlayerInfectionModule", &Offsets::Get_CreatePlayerInfectionModule, [](DWORD64 playerInfectionModule) {
			PlayerInfectionModule::EmplaceBack(reinterpret_cast<PlayerInfectionModule*>(playerInfectionModule));

			CreatePlayerInfectionModuleHook.ExecuteCallbacks(playerInfectionModule);
			return CreatePlayerInfectionModuleHook.pOriginal(playerInfectionModule);
		}};
#pragma endregion

#pragma region OnPostUpdate
		Utils::Hook::VTHook<GameDI_PH2*, void(*)(void*), void*> OnPostUpdateHook{ "OnPostUpdate", &GameDI_PH2::Get, [](void* pGameDI_PH2) {
			Core::OnPostUpdate();

			OnPostUpdateHook.ExecuteCallbacks(pGameDI_PH2);
			OnPostUpdateHook.pOriginal(pGameDI_PH2);
		}, 0x3A8 };
#pragma endregion
	}
}