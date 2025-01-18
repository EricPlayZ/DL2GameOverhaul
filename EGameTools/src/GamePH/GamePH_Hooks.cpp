#include <EGSDK\Offsets.h>
#include <EGSDK\Utils\Hook.h>
#include <EGSDK\GamePH\CoPlayerRestrictions.h>
#include <EGSDK\GamePH\FreeCamera.h>
#include <EGSDK\GamePH\GameDI_PH.h>
#include <EGSDK\GamePH\GameDI_PH2.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\GamePH\PlayerHealthModule.h>
#include <EGSDK\GamePH\PlayerInfectionModule.h>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGT\Menu\Camera.h>
#include <EGT\Menu\Misc.h>
#include <EGT\Menu\Player.h>
#include <EGT\Menu\Teleport.h>
#include <EGT\Menu\World.h>

namespace EGT::GamePH {
	namespace Hooks {
#pragma region LifeSetHealth
		static EGSDK::Utils::Hook::MHook<void*, void(*)(float*, float), float*, float> LifeSetHealthHook{ "LifeSetHealth", &EGSDK::Offsets::Get_LifeSetHealth, [](float* pLifeHealth, float health) -> void {
			if (!Menu::Player::godMode.GetValue() && !Menu::Camera::freeCam.GetValue())
				return LifeSetHealthHook.ExecuteCallbacksWithOriginal(pLifeHealth, health);

			auto playerHealthModule = EGSDK::GamePH::PlayerHealthModule::Get();
			if (!playerHealthModule)
				return LifeSetHealthHook.ExecuteCallbacksWithOriginal(pLifeHealth, health);
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return LifeSetHealthHook.ExecuteCallbacksWithOriginal(pLifeHealth, health);

			constexpr size_t addressThreshold = 0x100;
			if (std::abs(reinterpret_cast<LONG64>(playerHealthModule) - reinterpret_cast<LONG64>(pLifeHealth)) < addressThreshold && playerHealthModule->health > 0.0f)
				return;

			LifeSetHealthHook.ExecuteCallbacksWithOriginal(pLifeHealth, health);
		} };
#pragma endregion

#pragma region PlayerHealthModuleKillPlayer
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(void*), void*> PlayerHealthModuleKillPlayerHook{ "PlayerHealthModuleKillPlayer", &EGSDK::Offsets::Get_PlayerHealthModuleKillPlayer, [](void* playerHealthModule) -> DWORD64 {
			if (!Menu::Player::godMode.GetValue() && !Menu::Camera::freeCam.GetValue())
				return PlayerHealthModuleKillPlayerHook.ExecuteCallbacksWithOriginal(playerHealthModule);

			auto pPlayerHealthModule = EGSDK::GamePH::PlayerHealthModule::Get();
			if (!pPlayerHealthModule)
				return PlayerHealthModuleKillPlayerHook.ExecuteCallbacksWithOriginal(playerHealthModule);
			if (pPlayerHealthModule == playerHealthModule)
				return 0;

			return PlayerHealthModuleKillPlayerHook.ExecuteCallbacksWithOriginal(playerHealthModule);
		} };
#pragma endregion

#pragma region IsNotOutOfMapBounds
		static EGSDK::Utils::Hook::MHook<void*, bool(*)(void*, DWORD64), void*, DWORD64> IsNotOutOfMapBoundsHook{ "IsNotOutOfMapBounds", &EGSDK::Offsets::Get_IsNotOutOfMapBounds, [](void* pInstance, DWORD64 a2) -> bool {
			if (Menu::Player::disableOutOfBoundsTimer.GetValue())
				return true;

			return IsNotOutOfMapBoundsHook.ExecuteCallbacksWithOriginal(pInstance, a2);
		} };
#pragma endregion

#pragma region IsNotOutOfMissionBounds
		static EGSDK::Utils::Hook::MHook<void*, bool(*)(void*, DWORD64), void*, DWORD64> IsNotOutOfMissionBoundsHook{ "IsNotOutOfMissionBounds", &EGSDK::Offsets::Get_IsNotOutOfMissionBounds, [](void* pInstance, DWORD64 a2) -> bool {
			if (Menu::Player::disableOutOfBoundsTimer.GetValue())
				return true;

			return IsNotOutOfMissionBoundsHook.ExecuteCallbacksWithOriginal(pInstance, a2);
		} };
#pragma endregion

#pragma region ShowUIManager
		static void* GetShowUIManager() {
			return EGSDK::Utils::Memory::GetProcAddr("engine_x64_rwdi.dll", "?ShowUIManager@ILevel@@QEAAX_N@Z");
		}
		static EGSDK::Utils::Hook::MHook<void*, void(*)(void*, bool), void*, bool> ShowUIManagerHook{ "ShowUIManager", &GetShowUIManager, [](void* pLevelDI, bool enabled) -> void {
			if (Menu::Misc::disableHUD.GetValue())
				enabled = false;

			ShowUIManagerHook.ExecuteCallbacksWithOriginal(pLevelDI, enabled);
		} };
#pragma endregion

#pragma region TogglePhotoMode
		static EGSDK::Utils::Hook::MHook<void*, void(*)(void*, bool), void*, bool> TogglePhotoMode1Hook{ "TogglePhotoMode1", &EGSDK::Offsets::Get_TogglePhotoMode1, [](void* guiPhotoModeData, bool enabled) -> void {
			Menu::Camera::photoMode.Set(enabled);

			if (!Menu::Camera::freeCam.GetValue())
				return TogglePhotoMode1Hook.ExecuteCallbacksWithOriginal(guiPhotoModeData, enabled);
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || iLevel->IsTimerFrozen())
				return TogglePhotoMode1Hook.ExecuteCallbacksWithOriginal(guiPhotoModeData, enabled);
			auto pGameDI_PH = EGSDK::GamePH::GameDI_PH::Get();
			if (!pGameDI_PH)
				return TogglePhotoMode1Hook.ExecuteCallbacksWithOriginal(guiPhotoModeData, enabled);
			auto pFreeCam = EGSDK::GamePH::FreeCamera::Get();
			if (!pFreeCam)
				return TogglePhotoMode1Hook.ExecuteCallbacksWithOriginal(guiPhotoModeData, enabled);

			if (enabled) {
				pGameDI_PH->TogglePhotoMode();
				pFreeCam->AllowCameraMovement(0);
			}

			TogglePhotoMode1Hook.ExecuteCallbacksWithOriginal(guiPhotoModeData, enabled);
		} };
#pragma endregion

#pragma region ShowTPPModelFunc3
		ImGui::Option wannaUseTPPModel{};
		static bool prevUseTPPModel;

		static EGSDK::Utils::Hook::MHook<void*, void(*)(DWORD64, bool), DWORD64, bool> ShowTPPModelFunc3Hook{ "ShowTPPModelFunc3", &EGSDK::Offsets::Get_ShowTPPModelFunc3, [](DWORD64 tppFunc2Addr, bool showTPPModel) -> void {
			auto pPlayerDI_PH = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!pPlayerDI_PH)
				return ShowTPPModelFunc3Hook.ExecuteCallbacksWithOriginal(tppFunc2Addr, showTPPModel);

			if (!showTPPModel && prevUseTPPModel) {
				pPlayerDI_PH->enableTPPModel2 = true;
				pPlayerDI_PH->enableTPPModel1 = true;
			}
			ShowTPPModelFunc3Hook.ExecuteCallbacksWithOriginal(tppFunc2Addr, showTPPModel);
			if (showTPPModel && prevUseTPPModel) {
				pPlayerDI_PH->enableTPPModel2 = false;
				pPlayerDI_PH->enableTPPModel1 = false;
			} else
				prevUseTPPModel = showTPPModel;
		} };
#pragma endregion

#pragma region CalculateFreeCamCollision
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(void*, float*), void*, float*> CalculateFreeCamCollisionHook{ "CalculateFreeCamCollision", &EGSDK::Offsets::Get_CalculateFreeCamCollision, [](void* pFreeCamera, float* finalPos) -> DWORD64 {
			if (Menu::Camera::disablePhotoModeLimits.GetValue() || Menu::Camera::freeCam.GetValue())
				return 0;

			return CalculateFreeCamCollisionHook.ExecuteCallbacksWithOriginal(pFreeCamera, finalPos);
		} };
#pragma endregion

#pragma region PlaySoundEvent
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(void*, DWORD64, DWORD64), void*, DWORD64, DWORD64> PlaySoundEventHook{ "PlaySoundEvent", &EGSDK::Offsets::Get_PlaySoundEvent, [](void* pCoAudioEventControl, DWORD64 name, DWORD64 a3) -> DWORD64 {
			const char* soundName = reinterpret_cast<const char*>(name & 0x1FFFFFFFFFFFFFFF); // remove first byte of addr in case it exists
			if (Menu::World::freezeTime.GetValue() && soundName && (!strcmp(soundName, "set_gp_infection_start") || !strcmp(soundName, "set_gp_infection_immune")))
				return 0;

			return PlaySoundEventHook.ExecuteCallbacksWithOriginal(pCoAudioEventControl, name, a3);
		} };
#pragma endregion

#pragma region CalculateFallHeight
		static bool prevFreeCamValue = Menu::Camera::freeCam.GetPrevValue();

		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(void*, float), void*, float> CalculateFallHeightHook{ "CalculateFallHeight", &EGSDK::Offsets::Get_CalculateFallHeight, [](void* pInstance, float height) -> DWORD64 {
			prevFreeCamValue = Menu::Camera::freeCam.GetPrevValue();
			if (!Menu::Camera::freeCam.GetValue() && prevFreeCamValue) {
				Menu::Camera::freeCam.SetPrevValue(false);
				prevFreeCamValue = false;
				return 0;
			}
			if (Menu::Teleport::waypointIsSet && !*Menu::Teleport::waypointIsSet && Menu::Teleport::justTeleportedToWaypoint) {
				Menu::Teleport::justTeleportedToWaypoint = false;
				return 0;
			}

			if (Menu::Player::godMode.GetValue())
				return 0;

			return CalculateFallHeightHook.ExecuteCallbacksWithOriginal(pInstance, height);
		} };
#pragma endregion

#pragma region ReadPlayerJumpParams
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, DWORD64, DWORD64, char, DWORD64*), DWORD64, DWORD64, DWORD64, char, DWORD64*> ReadPlayerJumpParamsHook{ "ReadPlayerJumpParams", &EGSDK::Offsets::Get_ReadPlayerJumpParams, [](DWORD64 a1, DWORD64 a2, DWORD64 a3, char a4, DWORD64* a5) -> DWORD64 {
			DWORD64 result = ReadPlayerJumpParamsHook.ExecuteCallbacksWithOriginal(a1, a2, a3, a4, a5);

			if (Menu::Player::disableAirControl.GetValue())
				*reinterpret_cast<bool*>(a1 + EGSDK::Offsets::Get_allowVelocityMod_offset()) = false;
			if (Menu::Camera::disableHeadCorrection.GetValue() || Menu::Camera::goProMode.GetValue())
				*reinterpret_cast<bool*>(a1 + EGSDK::Offsets::Get_disableHeadCorrection_offset()) = true;

			return result;
		} };
#pragma endregion

#pragma region HandleInventoryItemsAmount
		static EGSDK::Utils::Hook::MHook<void*, void(*)(int*, UINT), int*, UINT> HandleInventoryItemsAmountHook{ "HandleInventoryItemsAmount", &EGSDK::Offsets::Get_HandleInventoryItemsAmount, [](int* pInventoryItem_0x10, UINT amount) -> void {
			int previousValue = *pInventoryItem_0x10;
			HandleInventoryItemsAmountHook.ExecuteCallbacksWithOriginal(pInventoryItem_0x10, amount);
			if (!EGSDK::GamePH::LevelDI::Get() || !EGSDK::GamePH::LevelDI::Get()->IsLoaded())
				return;

			if (*pInventoryItem_0x10 < previousValue && *pInventoryItem_0x10 == amount && Menu::Player::unlimitedItems.GetValue())
				*pInventoryItem_0x10 = previousValue;
		} };
#pragma endregion

#pragma region SetNewWaypointLocation
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, int, EGSDK::Vector3*), DWORD64, int, EGSDK::Vector3*> SetNewWaypointLocationHook{ "SetNewWaypointLocation", &EGSDK::Offsets::Get_SetNewWaypointLocation, [](DWORD64 pLogicalPlayer, int a2, EGSDK::Vector3* newWaypointLoc) -> DWORD64 {
			DWORD64 result = SetNewWaypointLocationHook.ExecuteCallbacksWithOriginal(pLogicalPlayer, a2, newWaypointLoc);
			Menu::Teleport::waypointCoords = *newWaypointLoc;
			if (EGSDK::Offsets::Get_SetNewWaypointLocationWaypointIsSetBoolInstr()) {
				const DWORD offset = *EGSDK::Offsets::Get_SetNewWaypointLocationWaypointIsSetBoolInstr();
				Menu::Teleport::waypointIsSet = reinterpret_cast<bool*>(pLogicalPlayer + offset);
			}
			return result;
		} };
#pragma endregion

#pragma region HandlePlayerRestrictions
		static EGSDK::Utils::Hook::MHook<void*, bool(*)(EGSDK::GamePH::PlayerDI_PH*), EGSDK::GamePH::PlayerDI_PH*> HandlePlayerRestrictionsHook{ "HandlePlayerRestrictions", &EGSDK::Offsets::Get_HandlePlayerRestrictions, [](EGSDK::GamePH::PlayerDI_PH* pPlayerDI_PH) -> bool {
			if (!pPlayerDI_PH)
				return HandlePlayerRestrictionsHook.ExecuteCallbacksWithOriginal(pPlayerDI_PH);
			if (!pPlayerDI_PH->areRestrictionsEnabledByGame)
				return HandlePlayerRestrictionsHook.ExecuteCallbacksWithOriginal(pPlayerDI_PH);

			if (Menu::Player::disableSafezoneRestrictions.GetValue())
				pPlayerDI_PH->restrictionsEnabled = false;
			else if (!Menu::Player::disableSafezoneRestrictions.GetValue())
				pPlayerDI_PH->restrictionsEnabled = true;

			return HandlePlayerRestrictionsHook.ExecuteCallbacksWithOriginal(pPlayerDI_PH);
		} };
#pragma endregion

#pragma region mEnablePlayerRestrictions
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(EGSDK::GamePH::CoPlayerRestrictions*), EGSDK::GamePH::CoPlayerRestrictions*> mEnablePlayerRestrictionsHook{ "mEnablePlayerRestrictions", &EGSDK::Offsets::Get_mEnablePlayerRestrictions, [](EGSDK::GamePH::CoPlayerRestrictions* pCoPlayerRestrictions) -> DWORD64 {
			EGSDK::GamePH::CoPlayerRestrictions::SetInstance(pCoPlayerRestrictions);
			auto playerDI_PH = EGSDK::GamePH::PlayerDI_PH::Get();
			if (playerDI_PH)
				playerDI_PH->areRestrictionsEnabledByGame = true;

			return mEnablePlayerRestrictionsHook.ExecuteCallbacksWithOriginal(pCoPlayerRestrictions);
		} };
#pragma endregion

#pragma region mDisablePlayerRestrictions
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(EGSDK::GamePH::CoPlayerRestrictions*), EGSDK::GamePH::CoPlayerRestrictions*> mDisablePlayerRestrictionsHook{ "mDisablePlayerRestrictions", &EGSDK::Offsets::Get_mDisablePlayerRestrictions, [](EGSDK::GamePH::CoPlayerRestrictions* pCoPlayerRestrictions) -> DWORD64 {
			EGSDK::GamePH::CoPlayerRestrictions::SetInstance(pCoPlayerRestrictions);
			auto playerDI_PH = EGSDK::GamePH::PlayerDI_PH::Get();
			if (playerDI_PH)
				playerDI_PH->areRestrictionsEnabledByGame = false;

			return mDisablePlayerRestrictionsHook.ExecuteCallbacksWithOriginal(pCoPlayerRestrictions);
		} };
#pragma endregion

#pragma region SetIsInCoSafeZone
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64*, UINT, DWORD64, bool, bool, bool), DWORD64*, UINT, DWORD64, bool, bool, bool> SetIsInCoSafeZoneHook{ "SetIsInCoSafeZone", &EGSDK::Offsets::Get_SetIsInCoSafeZone, [](DWORD64* pCoSafeZone, UINT a2, DWORD64 a3, bool a4, bool a5, bool a6) -> DWORD64 {
			if (Menu::Player::disableSafezoneRestrictions.GetValue())
				return 0;

			return SetIsInCoSafeZoneHook.ExecuteCallbacksWithOriginal(pCoSafeZone, a2, a3, a4, a5, a6);
		} };
#pragma endregion

#pragma region ByteHooks
		static unsigned char SaveGameCRCBoolCheckBytes[3] = { 0xB3, 0x01, 0x90 }; // mov bl, 01
		EGSDK::Utils::Hook::ByteHook<void*> SaveGameCRCBoolCheckHook{ "SaveGameCRCBoolCheck", &EGSDK::Offsets::Get_SaveGameCRCBoolCheck, SaveGameCRCBoolCheckBytes, sizeof(SaveGameCRCBoolCheckBytes) }; // and bl, dil
#pragma endregion
	}
}