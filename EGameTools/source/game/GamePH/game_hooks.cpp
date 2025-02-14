#include <pch.h>
#include "..\menu\camera.h"
#include "..\menu\misc.h"
#include "..\menu\player.h"
#include "..\menu\teleport.h"
#include "..\menu\world.h"
#include "..\offsets.h"
#include "FreeCamera.h"
#include "GameDI_PH.h"
#include "GameDI_PH2.h"
#include "LevelDI.h"
#include "PlayerHealthModule.h"
#include "PlayerInfectionModule.h"
#include "PlayerDI_PH.h"

namespace GamePH {
	namespace Hooks {
#pragma region CreatePlayerHealthModule
		static DWORD64 detourCreatePlayerHealthModule(DWORD64 playerHealthModule);
		static Utils::Hook::MHook<LPVOID, DWORD64(*)(DWORD64)> CreatePlayerHealthModuleHook{ "CreatePlayerHealthModule", &Offsets::Get_CreatePlayerHealthModule, &detourCreatePlayerHealthModule };

		static DWORD64 detourCreatePlayerHealthModule(DWORD64 playerHealthModule) {
			PlayerHealthModule::playerHealthModulePtrList.emplace_back(reinterpret_cast<PlayerHealthModule*>(playerHealthModule));

			return CreatePlayerHealthModuleHook.pOriginal(playerHealthModule);
		}
#pragma endregion

#pragma region CreatePlayerInfectionModule
		static DWORD64 detourCreatePlayerInfectionModule(DWORD64 playerInfectionModule);
		static Utils::Hook::MHook<LPVOID, DWORD64(*)(DWORD64)> CreatePlayerInfectionModuleHook{ "CreatePlayerInfectionModule", &Offsets::Get_CreatePlayerInfectionModule, &detourCreatePlayerInfectionModule };

		static DWORD64 detourCreatePlayerInfectionModule(DWORD64 playerInfectionModule) {
			PlayerInfectionModule::playerInfectionModulePtrList.emplace_back(reinterpret_cast<PlayerInfectionModule*>(playerInfectionModule));

			return CreatePlayerInfectionModuleHook.pOriginal(playerInfectionModule);
		}
#pragma endregion

#pragma region LifeSetHealth
		static void detourLifeSetHealth(float* pLifeHealth, float health);
		static Utils::Hook::MHook<LPVOID, void(*)(float*, float)> LifeSetHealthHook{ "LifeSetHealth", &Offsets::Get_LifeSetHealth, &detourLifeSetHealth };

		static void detourLifeSetHealth(float* pLifeHealth, float health) {
			if (!Menu::Player::godMode.GetValue() && !Menu::Camera::freeCam.GetValue())
				return LifeSetHealthHook.pOriginal(pLifeHealth, health);

			PlayerHealthModule* playerHealthModule = PlayerHealthModule::Get();
			if (!playerHealthModule)
				return LifeSetHealthHook.pOriginal(pLifeHealth, health);
			LevelDI* iLevel = LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return LifeSetHealthHook.pOriginal(pLifeHealth, health);

			if (std::abs(reinterpret_cast<LONG64>(playerHealthModule) - reinterpret_cast<LONG64>(pLifeHealth)) < 0x100 && playerHealthModule->health > 0.0f)
				return;

			LifeSetHealthHook.pOriginal(pLifeHealth, health);
		}
#pragma endregion

#pragma region PlayerHealthModuleKillPlayer
		static DWORD64 detourPlayerHealthModuleKillPlayer(LPVOID playerHealthModule);
		static Utils::Hook::MHook<LPVOID, DWORD64(*)(LPVOID)> PlayerHealthModuleKillPlayerHook{ "PlayerHealthModuleKillPlayer", &Offsets::Get_PlayerHealthModuleKillPlayer, &detourPlayerHealthModuleKillPlayer };

		static DWORD64 detourPlayerHealthModuleKillPlayer(LPVOID playerHealthModule) {
			if (!Menu::Player::godMode.GetValue() && !Menu::Camera::freeCam.GetValue())
				return PlayerHealthModuleKillPlayerHook.pOriginal(playerHealthModule);

			PlayerHealthModule* pPlayerHealthModule = PlayerHealthModule::Get();
			if (!pPlayerHealthModule)
				return PlayerHealthModuleKillPlayerHook.pOriginal(playerHealthModule);
			if (pPlayerHealthModule == playerHealthModule)
				return 0;

			return PlayerHealthModuleKillPlayerHook.pOriginal(playerHealthModule);
		}
#pragma endregion

#pragma region IsNotOutOfMapBounds
		static bool detourIsNotOutOfMapBounds(LPVOID pInstance, DWORD64 a2);
		static Utils::Hook::MHook<LPVOID, bool(*)(LPVOID, DWORD64)> IsNotOutOfMapBoundsHook{ "IsNotOutOfMapBounds", &Offsets::Get_IsNotOutOfMapBounds, &detourIsNotOutOfMapBounds };

		static bool detourIsNotOutOfMapBounds(LPVOID pInstance, DWORD64 a2) {
			if (Menu::Player::disableOutOfBoundsTimer.GetValue())
				return true;

			return IsNotOutOfMapBoundsHook.pOriginal(pInstance, a2);
		}
#pragma endregion

#pragma region IsNotOutOfMissionBounds
		static bool detourIsNotOutOfMissionBounds(LPVOID pInstance, DWORD64 a2);
		static Utils::Hook::MHook<LPVOID, bool(*)(LPVOID, DWORD64)> IsNotOutOfMissionBoundsHook{ "IsNotOutOfMissionBounds", &Offsets::Get_IsNotOutOfMissionBounds, &detourIsNotOutOfMissionBounds };

		static bool detourIsNotOutOfMissionBounds(LPVOID pInstance, DWORD64 a2) {
			if (Menu::Player::disableOutOfBoundsTimer.GetValue())
				return true;

			return IsNotOutOfMissionBoundsHook.pOriginal(pInstance, a2);
		}
#pragma endregion

#pragma region ShowUIManager
		static void detourShowUIManager(LPVOID pLevelDI, bool enabled);
		static LPVOID GetShowUIManager() {
			return Utils::Memory::GetProcAddr("engine_x64_rwdi.dll", "?ShowUIManager@ILevel@@QEAAX_N@Z");
		}
		static Utils::Hook::MHook<LPVOID, void(*)(LPVOID, bool)> ShowUIManagerHook{ "ShowUIManager", &GetShowUIManager, &detourShowUIManager };

		static void detourShowUIManager(LPVOID pLevelDI, bool enabled) {
			if (Menu::Misc::disableHUD.GetValue())
				enabled = false;

			ShowUIManagerHook.pOriginal(pLevelDI, enabled);
		}
#pragma endregion

#pragma region OnPostUpdate
		static void detourOnPostUpdate(LPVOID pGameDI_PH2);
		static Utils::Hook::VTHook<GamePH::GameDI_PH2*, void(*)(LPVOID)> OnPostUpdateHook{ "OnPostUpdate", &GamePH::GameDI_PH2::Get, &detourOnPostUpdate, 0x3A8 };

		static void detourOnPostUpdate(LPVOID pGameDI_PH2) {
			OnPostUpdateHook.pOriginal(pGameDI_PH2);
			Core::OnPostUpdate();
		}
#pragma endregion

#pragma region TogglePhotoMode
		static void detourTogglePhotoMode1(LPVOID guiPhotoModeData, bool enabled);
		static Utils::Hook::MHook<LPVOID, void(*)(LPVOID, bool)> TogglePhotoMode1Hook{ "TogglePhotoMode1", &Offsets::Get_TogglePhotoMode1, &detourTogglePhotoMode1 };

		static void detourTogglePhotoMode1(LPVOID guiPhotoModeData, bool enabled) {
			Menu::Camera::photoMode.Set(enabled);

			if (!Menu::Camera::freeCam.GetValue())
				return TogglePhotoMode1Hook.pOriginal(guiPhotoModeData, enabled);
			LevelDI* iLevel = LevelDI::Get();
			if (!iLevel || iLevel->IsTimerFrozen())
				return TogglePhotoMode1Hook.pOriginal(guiPhotoModeData, enabled);
			GameDI_PH* pGameDI_PH = GameDI_PH::Get();
			if (!pGameDI_PH)
				return TogglePhotoMode1Hook.pOriginal(guiPhotoModeData, enabled);
			FreeCamera* pFreeCam = FreeCamera::Get();
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
		Option wannaUseTPPModel{};
		static bool prevUseTPPModel;
		static void detourShowTPPModelFunc3(DWORD64 tppFunc2Addr, bool showTPPModel);
		static Utils::Hook::MHook<LPVOID, void(*)(DWORD64, bool)> ShowTPPModelFunc3Hook{ "ShowTPPModelFunc3", &Offsets::Get_ShowTPPModelFunc3, &detourShowTPPModelFunc3 };

		static void detourShowTPPModelFunc3(DWORD64 tppFunc2Addr, bool showTPPModel) {
			PlayerDI_PH* pPlayerDI_PH = PlayerDI_PH::Get();
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
		static DWORD64 detourCalculateFreeCamCollision(LPVOID pFreeCamera, float* finalPos);
		static Utils::Hook::MHook<LPVOID, DWORD64(*)(LPVOID, float*)> CalculateFreeCamCollisionHook{ "CalculateFreeCamCollision", &Offsets::Get_CalculateFreeCamCollision, &detourCalculateFreeCamCollision };

		static DWORD64 detourCalculateFreeCamCollision(LPVOID pFreeCamera, float* finalPos) {
			if (Menu::Camera::disablePhotoModeLimits.GetValue() || Menu::Camera::freeCam.GetValue())
				return 0;

			return CalculateFreeCamCollisionHook.pOriginal(pFreeCamera, finalPos);
		}
#pragma endregion

#pragma region PlaySoundEvent
		static DWORD64 detourPlaySoundEvent(LPVOID pCoAudioEventControl, DWORD64 name, DWORD64 a3);
		static Utils::Hook::MHook<LPVOID, DWORD64(*)(LPVOID, DWORD64, DWORD64)> PlaySoundEventHook{ "PlaySoundEvent", &Offsets::Get_PlaySoundEvent, &detourPlaySoundEvent };

		static DWORD64 detourPlaySoundEvent(LPVOID pCoAudioEventControl, DWORD64 name, DWORD64 a3) {
			const char* soundName = reinterpret_cast<const char*>(name & 0x1FFFFFFFFFFFFFFF); // remove first byte of addr in case it exists
			if (Menu::World::freezeTime.GetValue() && soundName &&
				(!strcmp(soundName, "set_gp_infection_start") || !strcmp(soundName, "set_gp_infection_immune"))) {
				return 0;
			}

			return PlaySoundEventHook.pOriginal(pCoAudioEventControl, name, a3);
		}
#pragma endregion

#pragma region CalculateFallHeight
		static DWORD64 detourCalculateFallHeight(LPVOID pInstance, float height);
		static Utils::Hook::MHook<LPVOID, DWORD64(*)(LPVOID, float)> CalculateFallHeightHook{ "CalculateFallHeight", &Offsets::Get_CalculateFallHeight, &detourCalculateFallHeight };

		static DWORD64 detourCalculateFallHeight(LPVOID pInstance, float height) {
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
		static bool detourCanUseGrappleHook(LPVOID pInstance, bool a2);
		static Utils::Hook::MHook<LPVOID, bool(*)(LPVOID, bool)> CanUseGrappleHookHook{ "CanUseGrappleHook", &Offsets::Get_CanUseGrappleHook, &detourCanUseGrappleHook };

		static bool detourCanUseGrappleHook(LPVOID pInstance, bool a2) {
			if (Menu::Player::allowGrappleHookInSafezone.GetValue())
				return true;

			return CanUseGrappleHookHook.pOriginal(pInstance, a2);
		}
#pragma endregion

#pragma region ReadPlayerJumpParams
		static DWORD64 detourReadPlayerJumpParams(DWORD64 a1, DWORD64 a2, DWORD64 a3, char a4, DWORD64* a5);
		static Utils::Hook::MHook<LPVOID, DWORD64(*)(DWORD64, DWORD64, DWORD64, char, DWORD64*)> ReadPlayerJumpParamsHook{ "ReadPlayerJumpParams", &Offsets::Get_ReadPlayerJumpParams, &detourReadPlayerJumpParams };

		static DWORD64 detourReadPlayerJumpParams(DWORD64 a1, DWORD64 a2, DWORD64 a3, char a4, DWORD64* a5) {
			DWORD64 result = ReadPlayerJumpParamsHook.pOriginal(a1, a2, a3, a4, a5);

			if (Menu::Player::disableAirControl.GetValue())
				*reinterpret_cast<bool*>(a1 + Offsets::Get_allowVelocityMod_offset()) = false;
			if (Menu::Camera::disableHeadCorrection.GetValue() || Menu::Camera::goProMode.GetValue())
				*reinterpret_cast<bool*>(a1 + Offsets::Get_disableHeadCorrection_offset()) = true;

			return result;
		}
#pragma endregion

#pragma region HandleInventoryItemsAmount
		static void detourHandleInventoryItemsAmount(int* pInventoryItem_0x10, UINT amount);
		static Utils::Hook::MHook<LPVOID, void(*)(int*, UINT)> HandleInventoryItemsAmountHook{ "HandleInventoryItemsAmount", &Offsets::Get_HandleInventoryItemsAmount, &detourHandleInventoryItemsAmount };

		static void detourHandleInventoryItemsAmount(int* pInventoryItem_0x10, UINT amount) {
			int previousValue = *pInventoryItem_0x10;
			HandleInventoryItemsAmountHook.pOriginal(pInventoryItem_0x10, amount);
			if (!LevelDI::Get() || !LevelDI::Get()->IsLoaded())
				return;

			if (*pInventoryItem_0x10 < previousValue && *pInventoryItem_0x10 == amount && Menu::Player::unlimitedItems.GetValue())
				*pInventoryItem_0x10 = previousValue;
		}
#pragma endregion

#pragma region SetNewWaypointLocation
		static DWORD64 detourSetNewWaypointLocation(DWORD64 pLogicalPlayer, int a2, Vector3* newWaypointLoc);
		static Utils::Hook::MHook<LPVOID, DWORD64(*)(DWORD64, int, Vector3*)> SetNewWaypointLocationHook{ "SetNewWaypointLocation", &Offsets::Get_SetNewWaypointLocation, &detourSetNewWaypointLocation };

		static DWORD64 detourSetNewWaypointLocation(DWORD64 pLogicalPlayer, int a2, Vector3* newWaypointLoc) {
			DWORD64 result = SetNewWaypointLocationHook.pOriginal(pLogicalPlayer, a2, newWaypointLoc);
			Menu::Teleport::waypointCoords = *newWaypointLoc;
			if (Offsets::Get_SetNewWaypointLocationWaypointIsSetBoolInstr()) {
				const DWORD offset = *Offsets::Get_SetNewWaypointLocationWaypointIsSetBoolInstr();
				Menu::Teleport::waypointIsSet = reinterpret_cast<bool*>(pLogicalPlayer + offset);
			}
			return result;
		}
#pragma endregion

/*#pragma region HandleHeadBob
		static bool isHandleHeadBobRunning = false;

		static void detourHandleHeadBob(DWORD64 a1, DWORD64 a2, DWORD64 a3, DWORD64 a4);
		static Utils::Hook::MHook<LPVOID, void(*)(DWORD64, DWORD64, DWORD64, DWORD64)> HandleHeadBobHook{ "HandleHeadBob", &Offsets::Get_HandleHeadBob, &detourHandleHeadBob };

		static void detourHandleHeadBob(DWORD64 a1, DWORD64 a2, DWORD64 a3, DWORD64 a4) {
			isHandleHeadBobRunning = true;
			HandleHeadBobHook.pOriginal(a1, a2, a3, a4);
			isHandleHeadBobRunning = false;
		}
#pragma endregion

#pragma region SomeFloatCalcFunc
		static DWORD64 detourSomeFloatCalcFunc(float* a1, float* a2, float a3, DWORD64 a4, DWORD64 a5, DWORD64 a6);
		static Utils::Hook::MHook<LPVOID, DWORD64(*)(float*, float*, float, DWORD64, DWORD64, DWORD64)> SomeFloatCalcFuncHook{ "SomeFloatCalcFunc", &Offsets::Get_SomeFloatCalcFunc, &detourSomeFloatCalcFunc };

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
		Utils::Hook::ByteHook<LPVOID> SaveGameCRCBoolCheckHook{ "SaveGameCRCBoolCheck", &Offsets::Get_SaveGameCRCBoolCheck, SaveGameCRCBoolCheckBytes, sizeof(SaveGameCRCBoolCheckBytes), &Menu::Misc::disableSavegameCRCCheck }; // and bl, dil
#pragma endregion
	}
}