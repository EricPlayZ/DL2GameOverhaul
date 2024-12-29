#include <EGSDK\Offsets.h>
#include <EGSDK\Utils\Hook.h>
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
		static void detourLifeSetHealth(float* pLifeHealth, float health);
		static EGSDK::Utils::Hook::MHook<void*, void(*)(float*, float)> LifeSetHealthHook{ "LifeSetHealth", &EGSDK::Offsets::Get_LifeSetHealth, &detourLifeSetHealth };

		static void detourLifeSetHealth(float* pLifeHealth, float health) {
			if (!Menu::Player::godMode.GetValue() && !Menu::Camera::freeCam.GetValue())
				return LifeSetHealthHook.pOriginal(pLifeHealth, health);

			EGSDK::GamePH::PlayerHealthModule* playerHealthModule = EGSDK::GamePH::PlayerHealthModule::Get();
			if (!playerHealthModule)
				return LifeSetHealthHook.pOriginal(pLifeHealth, health);
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return LifeSetHealthHook.pOriginal(pLifeHealth, health);

			if (std::abs(reinterpret_cast<LONG64>(playerHealthModule) - reinterpret_cast<LONG64>(pLifeHealth)) < 0x100 && playerHealthModule->health > 0.0f)
				return;

			LifeSetHealthHook.pOriginal(pLifeHealth, health);
		}
#pragma endregion

#pragma region PlayerHealthModuleKillPlayer
		static DWORD64 detourPlayerHealthModuleKillPlayer(void* playerHealthModule);
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(void*)> PlayerHealthModuleKillPlayerHook{ "PlayerHealthModuleKillPlayer", &EGSDK::Offsets::Get_PlayerHealthModuleKillPlayer, &detourPlayerHealthModuleKillPlayer };

		static DWORD64 detourPlayerHealthModuleKillPlayer(void* playerHealthModule) {
			if (!Menu::Player::godMode.GetValue() && !Menu::Camera::freeCam.GetValue())
				return PlayerHealthModuleKillPlayerHook.pOriginal(playerHealthModule);

			EGSDK::GamePH::PlayerHealthModule* pPlayerHealthModule = EGSDK::GamePH::PlayerHealthModule::Get();
			if (!pPlayerHealthModule)
				return PlayerHealthModuleKillPlayerHook.pOriginal(playerHealthModule);
			if (pPlayerHealthModule == playerHealthModule)
				return 0;

			return PlayerHealthModuleKillPlayerHook.pOriginal(playerHealthModule);
		}
#pragma endregion

#pragma region IsNotOutOfMapBounds
		static bool detourIsNotOutOfMapBounds(void* pInstance, DWORD64 a2);
		static EGSDK::Utils::Hook::MHook<void*, bool(*)(void*, DWORD64)> IsNotOutOfMapBoundsHook{ "IsNotOutOfMapBounds", &EGSDK::Offsets::Get_IsNotOutOfMapBounds, &detourIsNotOutOfMapBounds };

		static bool detourIsNotOutOfMapBounds(void* pInstance, DWORD64 a2) {
			if (Menu::Player::disableOutOfBoundsTimer.GetValue())
				return true;

			return IsNotOutOfMapBoundsHook.pOriginal(pInstance, a2);
		}
#pragma endregion

#pragma region IsNotOutOfMissionBounds
		static bool detourIsNotOutOfMissionBounds(void* pInstance, DWORD64 a2);
		static EGSDK::Utils::Hook::MHook<void*, bool(*)(void*, DWORD64)> IsNotOutOfMissionBoundsHook{ "IsNotOutOfMissionBounds", &EGSDK::Offsets::Get_IsNotOutOfMissionBounds, &detourIsNotOutOfMissionBounds };

		static bool detourIsNotOutOfMissionBounds(void* pInstance, DWORD64 a2) {
			if (Menu::Player::disableOutOfBoundsTimer.GetValue())
				return true;

			return IsNotOutOfMissionBoundsHook.pOriginal(pInstance, a2);
		}
#pragma endregion

#pragma region ShowUIManager
		static void detourShowUIManager(void* pLevelDI, bool enabled);
		static void* GetShowUIManager() {
			return EGSDK::Utils::Memory::GetProcAddr("engine_x64_rwdi.dll", "?ShowUIManager@ILevel@@QEAAX_N@Z");
		}
		static EGSDK::Utils::Hook::MHook<void*, void(*)(void*, bool)> ShowUIManagerHook{ "ShowUIManager", &GetShowUIManager, &detourShowUIManager };

		static void detourShowUIManager(void* pLevelDI, bool enabled) {
			if (Menu::Misc::disableHUD.GetValue())
				enabled = false;

			ShowUIManagerHook.pOriginal(pLevelDI, enabled);
		}
#pragma endregion

#pragma region TogglePhotoMode
		static void detourTogglePhotoMode1(void* guiPhotoModeData, bool enabled);
		static EGSDK::Utils::Hook::MHook<void*, void(*)(void*, bool)> TogglePhotoMode1Hook{ "TogglePhotoMode1", &EGSDK::Offsets::Get_TogglePhotoMode1, &detourTogglePhotoMode1 };

		static void detourTogglePhotoMode1(void* guiPhotoModeData, bool enabled) {
			Menu::Camera::photoMode.Set(enabled);

			if (!Menu::Camera::freeCam.GetValue())
				return TogglePhotoMode1Hook.pOriginal(guiPhotoModeData, enabled);
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || iLevel->IsTimerFrozen())
				return TogglePhotoMode1Hook.pOriginal(guiPhotoModeData, enabled);
			EGSDK::GamePH::GameDI_PH* pGameDI_PH = EGSDK::GamePH::GameDI_PH::Get();
			if (!pGameDI_PH)
				return TogglePhotoMode1Hook.pOriginal(guiPhotoModeData, enabled);
			EGSDK::GamePH::FreeCamera* pFreeCam = EGSDK::GamePH::FreeCamera::Get();
			if (!pFreeCam)
				return TogglePhotoMode1Hook.pOriginal(guiPhotoModeData, enabled);

			if (enabled) {
				pGameDI_PH->TogglePhotoMode();
				pFreeCam->AllowCameraMovement(0);
			}

			TogglePhotoMode1Hook.pOriginal(guiPhotoModeData, enabled);
		}
#pragma endregion

#pragma region ShowTPPModelFunc3
		ImGui::Option wannaUseTPPModel{};
		static bool prevUseTPPModel;
		static void detourShowTPPModelFunc3(DWORD64 tppFunc2Addr, bool showTPPModel);
		static EGSDK::Utils::Hook::MHook<void*, void(*)(DWORD64, bool)> ShowTPPModelFunc3Hook{ "ShowTPPModelFunc3", &EGSDK::Offsets::Get_ShowTPPModelFunc3, &detourShowTPPModelFunc3 };

		static void detourShowTPPModelFunc3(DWORD64 tppFunc2Addr, bool showTPPModel) {
			EGSDK::GamePH::PlayerDI_PH* pPlayerDI_PH = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!pPlayerDI_PH) {
				ShowTPPModelFunc3Hook.pOriginal(tppFunc2Addr, showTPPModel);
				return;
			}
			
			if (!showTPPModel && prevUseTPPModel) {
				pPlayerDI_PH->enableTPPModel2 = true;
				pPlayerDI_PH->enableTPPModel1 = true;
			}
			ShowTPPModelFunc3Hook.pOriginal(tppFunc2Addr, showTPPModel);
			if (showTPPModel && prevUseTPPModel) {
				pPlayerDI_PH->enableTPPModel2 = false;
				pPlayerDI_PH->enableTPPModel1 = false;
			} else
				prevUseTPPModel = showTPPModel;
		}
#pragma endregion

#pragma region CalculateFreeCamCollision
		static DWORD64 detourCalculateFreeCamCollision(void* pFreeCamera, float* finalPos);
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(void*, float*)> CalculateFreeCamCollisionHook{ "CalculateFreeCamCollision", &EGSDK::Offsets::Get_CalculateFreeCamCollision, &detourCalculateFreeCamCollision };

		static DWORD64 detourCalculateFreeCamCollision(void* pFreeCamera, float* finalPos) {
			if (Menu::Camera::disablePhotoModeLimits.GetValue() || Menu::Camera::freeCam.GetValue())
				return 0;

			return CalculateFreeCamCollisionHook.pOriginal(pFreeCamera, finalPos);
		}
#pragma endregion

#pragma region PlaySoundEvent
		static DWORD64 detourPlaySoundEvent(void* pCoAudioEventControl, DWORD64 name, DWORD64 a3);
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(void*, DWORD64, DWORD64)> PlaySoundEventHook{ "PlaySoundEvent", &EGSDK::Offsets::Get_PlaySoundEvent, &detourPlaySoundEvent };

		static DWORD64 detourPlaySoundEvent(void* pCoAudioEventControl, DWORD64 name, DWORD64 a3) {
			const char* soundName = reinterpret_cast<const char*>(name & 0x1FFFFFFFFFFFFFFF); // remove first byte of addr in case it exists
			if (Menu::World::freezeTime.GetValue() && soundName &&
				(!strcmp(soundName, "set_gp_infection_start") || !strcmp(soundName, "set_gp_infection_immune"))) {
				return 0;
			}

			return PlaySoundEventHook.pOriginal(pCoAudioEventControl, name, a3);
		}
#pragma endregion

#pragma region CalculateFallHeight
		static DWORD64 detourCalculateFallHeight(void* pInstance, float height);
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(void*, float)> CalculateFallHeightHook{ "CalculateFallHeight", &EGSDK::Offsets::Get_CalculateFallHeight, &detourCalculateFallHeight };

		static DWORD64 detourCalculateFallHeight(void* pInstance, float height) {
			static bool prevFreeCam = Menu::Camera::freeCam.GetPrevValue();
			prevFreeCam = Menu::Camera::freeCam.GetPrevValue();
			if (!Menu::Camera::freeCam.GetValue() && prevFreeCam) {
				Menu::Camera::freeCam.SetPrevValue(false);
				prevFreeCam = false;
				return 0;
			}
			if (Menu::Teleport::waypointIsSet && !*Menu::Teleport::waypointIsSet && Menu::Teleport::justTeleportedToWaypoint) {
				Menu::Teleport::justTeleportedToWaypoint = false;
				return 0;
			}

			if (Menu::Player::godMode.GetValue())
				return 0;

			return CalculateFallHeightHook.pOriginal(pInstance, height);
		}
#pragma endregion

#pragma region CanUseGrappleHook
		static bool detourCanUseGrappleHook(void* pInstance, bool a2);
		static EGSDK::Utils::Hook::MHook<void*, bool(*)(void*, bool)> CanUseGrappleHookHook{ "CanUseGrappleHook", &EGSDK::Offsets::Get_CanUseGrappleHook, &detourCanUseGrappleHook };

		static bool detourCanUseGrappleHook(void* pInstance, bool a2) {
			if (Menu::Player::allowGrappleHookInSafezone.GetValue())
				return true;

			return CanUseGrappleHookHook.pOriginal(pInstance, a2);
		}
#pragma endregion

#pragma region ReadPlayerJumpParams
		static DWORD64 detourReadPlayerJumpParams(DWORD64 a1, DWORD64 a2, DWORD64 a3, char a4, DWORD64* a5);
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, DWORD64, DWORD64, char, DWORD64*)> ReadPlayerJumpParamsHook{ "ReadPlayerJumpParams", &EGSDK::Offsets::Get_ReadPlayerJumpParams, &detourReadPlayerJumpParams };

		static DWORD64 detourReadPlayerJumpParams(DWORD64 a1, DWORD64 a2, DWORD64 a3, char a4, DWORD64* a5) {
			DWORD64 result = ReadPlayerJumpParamsHook.pOriginal(a1, a2, a3, a4, a5);

			if (Menu::Player::disableAirControl.GetValue())
				*reinterpret_cast<bool*>(a1 + EGSDK::Offsets::Get_allowVelocityMod_offset()) = false;
			if (Menu::Camera::disableHeadCorrection.GetValue() || Menu::Camera::goProMode.GetValue())
				*reinterpret_cast<bool*>(a1 + EGSDK::Offsets::Get_disableHeadCorrection_offset()) = true;

			return result;
		}
#pragma endregion

#pragma region HandleInventoryItemsAmount
		static void detourHandleInventoryItemsAmount(int* pInventoryItem_0x10, UINT amount);
		static EGSDK::Utils::Hook::MHook<void*, void(*)(int*, UINT)> HandleInventoryItemsAmountHook{ "HandleInventoryItemsAmount", &EGSDK::Offsets::Get_HandleInventoryItemsAmount, &detourHandleInventoryItemsAmount };

		static void detourHandleInventoryItemsAmount(int* pInventoryItem_0x10, UINT amount) {
			int previousValue = *pInventoryItem_0x10;
			HandleInventoryItemsAmountHook.pOriginal(pInventoryItem_0x10, amount);
			if (!EGSDK::GamePH::LevelDI::Get() || !EGSDK::GamePH::LevelDI::Get()->IsLoaded())
				return;

			if (*pInventoryItem_0x10 < previousValue && *pInventoryItem_0x10 == amount && Menu::Player::unlimitedItems.GetValue())
				*pInventoryItem_0x10 = previousValue;
		}
#pragma endregion

#pragma region SetNewWaypointLocation
		static DWORD64 detourSetNewWaypointLocation(DWORD64 pLogicalPlayer, int a2, EGSDK::Vector3* newWaypointLoc);
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, int, EGSDK::Vector3*)> SetNewWaypointLocationHook{ "SetNewWaypointLocation", &EGSDK::Offsets::Get_SetNewWaypointLocation, &detourSetNewWaypointLocation };

		static DWORD64 detourSetNewWaypointLocation(DWORD64 pLogicalPlayer, int a2, EGSDK::Vector3* newWaypointLoc) {
			DWORD64 result = SetNewWaypointLocationHook.pOriginal(pLogicalPlayer, a2, newWaypointLoc);
			Menu::Teleport::waypointCoords = *newWaypointLoc;
			if (EGSDK::Offsets::Get_SetNewWaypointLocationWaypointIsSetBoolInstr()) {
				const DWORD offset = *EGSDK::Offsets::Get_SetNewWaypointLocationWaypointIsSetBoolInstr();
				Menu::Teleport::waypointIsSet = reinterpret_cast<bool*>(pLogicalPlayer + offset);
			}
			return result;
		}
#pragma endregion

/*#pragma region HandleHeadBob
		static bool isHandleHeadBobRunning = false;

		static void detourHandleHeadBob(DWORD64 a1, DWORD64 a2, DWORD64 a3, DWORD64 a4);
		static EGSDK::Utils::Hook::MHook<void*, void(*)(DWORD64, DWORD64, DWORD64, DWORD64)> HandleHeadBobHook{ "HandleHeadBob", &EGSDK::OffsetsGet_HandleHeadBob, &detourHandleHeadBob };

		static void detourHandleHeadBob(DWORD64 a1, DWORD64 a2, DWORD64 a3, DWORD64 a4) {
			isHandleHeadBobRunning = true;
			HandleHeadBobHook.pOriginal(a1, a2, a3, a4);
			isHandleHeadBobRunning = false;
		}
#pragma endregion

#pragma region SomeFloatCalcFunc
		static DWORD64 detourSomeFloatCalcFunc(float* a1, float* a2, float a3, DWORD64 a4, DWORD64 a5, DWORD64 a6);
		static EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(float*, float*, float, DWORD64, DWORD64, DWORD64)> SomeFloatCalcFuncHook{ "SomeFloatCalcFunc", &EGSDK::OffsetsGet_SomeFloatCalcFunc, &detourSomeFloatCalcFunc };

		static DWORD64 detourSomeFloatCalcFunc(float* a1, float* a2, float a3, DWORD64 a4, DWORD64 a5, DWORD64 a6) {
			if (isHandleHeadBobRunning) {
				static int i = 1;
				if (*a1 < 0.002f && i >= 2 && i <= 3) {
					*a1 *= 2.0f;
					a3 *= 2.0f;
				}
				i++;
				if (i > 3)
					i = 1;
			}
			return SomeFloatCalcFuncHook.pOriginal(a1, a2, a3, a4, a5, a6);
		}
#pragma endregion*/

#pragma region ByteHooks
		static unsigned char SaveGameCRCBoolCheckBytes[3] = { 0xB3, 0x01, 0x90 }; // mov bl, 01
		EGSDK::Utils::Hook::ByteHook<void*> SaveGameCRCBoolCheckHook{ "SaveGameCRCBoolCheck", &EGSDK::Offsets::Get_SaveGameCRCBoolCheck, SaveGameCRCBoolCheckBytes, sizeof(SaveGameCRCBoolCheckBytes) }; // and bl, dil
#pragma endregion
	}
}