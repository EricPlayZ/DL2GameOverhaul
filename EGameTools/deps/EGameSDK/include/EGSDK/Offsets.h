#pragma once
#include <EGSDK\Utils\Memory.h>
#include <EGSDK\Utils\RTTI.h>
#include <EGSDK\Utils\Sigscan.h>

namespace EGSDK {
	#define AddOffset(name, moduleName, pattern, type, retType)\
	static retType Get_## name () {\
		static retType name = NULL;\
		static int i = 0;\
		if (Utils::Memory::IsValidPtr(name) || !GetModuleHandleA(moduleName) || i >= 50) return name;\
		i++;\
		return name=reinterpret_cast<retType>(Utils::SigScan::PatternScanner::FindPattern(moduleName, {pattern, type}));\
	} 
	#define AddStaticOffset(name, off)\
	static DWORD64 Get_## name () {\
		static DWORD64 name = 0;\
		if (name) return name; \
		return name=static_cast<DWORD64>(off);\
	} 
	#define AddStaticOffset2(name, moduleName, off) \
	static DWORD64 Get_## name () {\
		static DWORD64 name = 0;\
		if (Utils::Memory::IsValidPtr(name)) return name;\
		return name=reinterpret_cast<DWORD64>(GetModuleHandleA(moduleName)) + static_cast<DWORD64>(off);\
	}

	#define AddVTOffset(name, moduleName, rttiName, retType)\
	static retType GetVT_## name () {\
		static retType VT_## name = NULL;\
		static int i = 0;\
		if (Utils::Memory::IsValidPtr(VT_## name)|| !GetModuleHandleA(moduleName) || i >= 50) return VT_## name;\
		i++;\
		return VT_## name=reinterpret_cast<retType>(Utils::RTTI::GetVTablePtr(moduleName, rttiName));\
	} 

	struct Offsets {
		// Input related
		AddOffset(CInput, "engine_x64_rwdi.dll", "48 8B 0D [?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 01 84 D2", Utils::SigScan::PatternType::RelativePointer, DWORD64**) // g_CInput

		// Player vars related
		AddVTOffset(FloatPlayerVariable, "gamedll_ph_x64_rwdi.dll", "FloatPlayerVariable", void*)
		AddVTOffset(BoolPlayerVariable, "gamedll_ph_x64_rwdi.dll", "BoolPlayerVariable", void*)
		AddVTOffset(TypedFieldMetaFloatPlayerVariable, "gamedll_ph_x64_rwdi.dll", "?$TypedFieldMeta@VFloatPlayerVariable@@@?$FieldsCollection@VPlayerVariables@@@constds", void*)
		AddVTOffset(TypedFieldMetaBoolPlayerVariable, "gamedll_ph_x64_rwdi.dll", "?$TypedFieldMeta@VBoolPlayerVariable@@@?$FieldsCollection@VPlayerVariables@@@constds", void*)
		AddOffset(LoadPlayerVars, "gamedll_ph_x64_rwdi.dll", "48 89 4C 24 ?? B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 2B E0 48 8B 8C 24", Utils::SigScan::PatternType::Address, void*)
		AddOffset(PlayerState, "gamedll_ph_x64_rwdi.dll", "48 8B 35 [?? ?? ?? ?? 4C 8B F2 48 8B F9", Utils::SigScan::PatternType::RelativePointer, void*)

		// Game related
		AddVTOffset(CBulletPhysicsCharacter, "engine_x64_rwdi.dll", "CBulletPhysicsCharacter", void*)
		AddVTOffset(CGSObject, "engine_x64_rwdi.dll", "CGSObject", void*)
		AddVTOffset(CInput, "engine_x64_rwdi.dll", "CInput", void*)
		AddVTOffset(CLevel, "engine_x64_rwdi.dll", "CLevel", void*)
		AddVTOffset(CLobbySteam, "engine_x64_rwdi.dll", "CLobbySteam", void*)
		AddVTOffset(CVideoSettings, "engine_x64_rwdi.dll", "CVideoSettings", void*)

		AddVTOffset(DayNightCycle, "gamedll_ph_x64_rwdi.dll", "DayNightCycle", void*)
		AddVTOffset(FreeCamera, "gamedll_ph_x64_rwdi.dll", "FreeCamera", void*)
		AddVTOffset(GameDI_PH, "gamedll_ph_x64_rwdi.dll", "GameDI_PH", void*)
		AddVTOffset(InventoryContainerDI, "gamedll_ph_x64_rwdi.dll", "InventoryContainerDI", void*)
		AddVTOffset(InventoryItem, "gamedll_ph_x64_rwdi.dll", "InventoryItem", void*)
		AddVTOffset(InventoryMoney, "gamedll_ph_x64_rwdi.dll", "InventoryMoney", void*)
		AddVTOffset(ItemDescWithContext, "gamedll_ph_x64_rwdi.dll", "ItemDescWithContext", void*)
		AddVTOffset(LevelDI, "gamedll_ph_x64_rwdi.dll", "LevelDI", void*)
		AddVTOffset(LocalClientDI, "gamedll_ph_x64_rwdi.dll", "LocalClientDI", void*)
		AddVTOffset(LogicalPlayer, "gamedll_ph_x64_rwdi.dll", "LogicalPlayer", void*)
		AddVTOffset(PlayerDI_PH, "gamedll_ph_x64_rwdi.dll", "PlayerDI_PH", void*)
		AddVTOffset(PlayerState, "gamedll_ph_x64_rwdi.dll", "PlayerState", void*)
		AddVTOffset(SessionCooperativeDI, "gamedll_ph_x64_rwdi.dll", "SessionCooperativeDI", void*)
		AddVTOffset(TPPCameraDI, "gamedll_ph_x64_rwdi.dll", "TPPCameraDI", void*)

		AddStaticOffset(gameDI_PH2_offset, 0x28)
		AddStaticOffset(allowVelocityMod_offset, 0x5C)
		AddStaticOffset(disableHeadCorrection_offset, 0x108)
		AddOffset(CLobbySteam, "engine_x64_rwdi.dll", "48 8B 05 [?? ?? ?? ?? 48 85 C0 74 ?? 48 83 C0", Utils::SigScan::PatternType::RelativePointer, void*)
		//AddOffset(g_PlayerDI_PH, "gamedll_ph_x64_rwdi.dll", "48 89 0D [?? ?? ?? ?? E8 ?? ?? ?? ?? 48 85 C0", Utils::SigScan::PatternType::RelativePointer, void*) // also PlayerObjProperties
		AddOffset(DayNightCycle, "gamedll_ph_x64_rwdi.dll", "48 8B 0D [?? ?? ?? ?? 48 85 C9 74 ?? E8 ?? ?? ?? ?? 84 C0 74 ?? B0 ?? 48 83 C4 ?? C3 32 C0", Utils::SigScan::PatternType::RelativePointer, void*) // g_DayNightCycle
		//AddOffset(g_CameraFPPDI, "gamedll_ph_x64_rwdi.dll", "48 89 05 [?? ?? ?? ?? 40 84 FF", PatternType::RelativePointer, DWORD64*)
		AddOffset(FreeCamera, "gamedll_ph_x64_rwdi.dll", "48 89 05 [?? ?? ?? ?? 48 89 4C 24", Utils::SigScan::PatternType::RelativePointer, DWORD64*) // g_FreeCamera
		AddOffset(SaveGameCRCBoolCheck, "gamedll_ph_x64_rwdi.dll", "FF 50 ?? [40 22 DF 0F 85 ?? ?? ?? ?? 0F B6 05 ?? ?? ?? ?? 48 8D 3D", Utils::SigScan::PatternType::Address, void*)

		// Functions
		AddOffset(ReadVideoSettings, "engine_x64_rwdi.dll", "48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 48 8B FA 48 8B D9 45 84 C0", Utils::SigScan::PatternType::Address, void*)
		AddOffset(MountDataPaks, "engine_x64_rwdi.dll", "4C 8B DC 4D 89 4B ?? 45 89 43 ?? 89 54 24 ?? 49 89 4B", Utils::SigScan::PatternType::Address, void*)
		AddOffset(MoveCameraFromForwardUpPos, "engine_x64_rwdi.dll", "48 89 5C 24 ?? 57 48 83 EC ?? 49 8B C1 48 8B F9", Utils::SigScan::PatternType::Address, void*)
		AddOffset(CalculateFreeCamCollision, "gamedll_ph_x64_rwdi.dll", "48 8B C4 55 53 56 57 48 8D A8 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 83 B9", Utils::SigScan::PatternType::Address, void*)
		AddOffset(AllowCameraMovement, "gamedll_ph_x64_rwdi.dll", "89 91 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC CC 48 8B C4 55 56", Utils::SigScan::PatternType::Address, void*)
		//AddOffset(GetViewCamera, "engine_x64_rwdi.dll", "E8 [?? ?? ?? ?? 48 85 C0 74 28 48 8B C8", PatternType::RelativePointer, void*)
		AddOffset(CreatePlayerHealthModule, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ?? 48 81 EC ?? ?? ?? ?? 4C 8B F1 E8 ?? ?? ?? ?? 48 8D 05", Utils::SigScan::PatternType::Address, void*)
		AddOffset(CreatePlayerInfectionModule, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 57 48 83 EC ?? 48 8B D9 E8 ?? ?? ?? ?? 33 FF 66 C7 44 24 ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 89 03", Utils::SigScan::PatternType::Address, void*)
		AddOffset(LifeSetHealth, "gamedll_ph_x64_rwdi.dll", "F3 0F 11 49 ?? F3 0F 11 49 ?? F3 0F 11 49 ?? C3", Utils::SigScan::PatternType::Address, void*)
		AddOffset(TogglePhotoMode1, "gamedll_ph_x64_rwdi.dll", "48 83 EC ?? 38 91 ?? ?? ?? ?? 0F 84", Utils::SigScan::PatternType::Address, void*)
		AddOffset(TogglePhotoMode2, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 57 48 83 EC ?? 48 8B D9 41 0F B6 F8 48 8B 0D", Utils::SigScan::PatternType::Address, void*)
		//AddOffset(OnUpdate_ChangeMap, "gamedll_ph_x64_rwdi.dll", "E8 [?? ?? ?? ?? 88 44 24 20 48 8B 84 24 ?? ?? ?? ?? 48 83 78 ?? ??", PatternType::RelativePointer, void*)
		AddOffset(ShowTPPModelFunc2, "gamedll_ph_x64_rwdi.dll", "40 53 48 83 EC ?? 48 8B 01 48 8B D9 FF 90 ?? ?? ?? ?? 84 C0 74 ?? 48 8B 83 ?? ?? ?? ?? 48 8B 88 ?? ?? ?? ?? 48 85 C9 74 ?? 48 83 B9 ?? ?? ?? ?? ?? 74 ?? 48 8B 81", Utils::SigScan::PatternType::Address, void*)
		AddOffset(ShowTPPModelFunc3, "gamedll_ph_x64_rwdi.dll", "48 83 EC ?? 38 91 ?? ?? ?? ?? 74 ?? 88 91", Utils::SigScan::PatternType::Address, void*)
		//AddOffset(CalculateOutOfBoundsTimer, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 81 EC ?? ?? ?? ?? 0F B6 99", PatternType::Address, void*)
		AddOffset(IsNotOutOfMapBounds, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 41 56 41 57 48 83 EC ?? 4C 8B F9 48 85 D2", Utils::SigScan::PatternType::Address, void*)
		AddOffset(IsNotOutOfMissionBounds, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 57 48 83 EC ?? 48 8B F9 48 85 D2 74 ?? 48 8D 8A", Utils::SigScan::PatternType::Address, void*)
		AddOffset(ReloadJumps, "gamedll_ph_x64_rwdi.dll", "48 83 EC ?? E8 ?? ?? ?? ?? 48 8D 15", Utils::SigScan::PatternType::Address, void*)
		AddOffset(PlaySoundEvent, "gamedll_ph_x64_rwdi.dll", "4C 8B DC 49 89 5B ?? 49 89 73 ?? 57 48 81 EC ?? ?? ?? ?? 48 8B 44 24 ?? 48 8B F9 48 8B DA", Utils::SigScan::PatternType::Address, void*)
		AddOffset(CalculateFallHeight, "gamedll_ph_x64_rwdi.dll", "40 55 56 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 44 0F 29 9C 24", Utils::SigScan::PatternType::Address, void*)
		AddOffset(PlayerHealthModuleKillPlayer, "gamedll_ph_x64_rwdi.dll", "40 53 48 83 EC ?? 48 8B 01 48 8B D9 FF 90 ?? ?? ?? ?? 84 C0 74 ?? 48 8B 4B ?? 48 81 C1 ?? ?? ?? ?? 48 8B 01 FF 50", Utils::SigScan::PatternType::Address, void*)
		AddOffset(CanUseGrappleHook, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 57 48 83 EC ?? 48 8B 01 0F B6 FA 48 8B D9 FF 90 ?? ?? ?? ?? F6 80", Utils::SigScan::PatternType::Address, void*)
		AddOffset(ReadPlayerJumpParams, "gamedll_ph_x64_rwdi.dll", "40 55 56 57 41 56 41 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 4C 8B B5", Utils::SigScan::PatternType::Address, void*)
		AddOffset(PlayerGetCurrentWeapon, "gamedll_ph_x64_rwdi.dll", "8B C2 48 8D 14 80 48 83 BC D1 ?? ?? ?? ?? ?? 74 ?? 48 8B 84 D1 ?? ?? ?? ?? C3 33 C0 C3 CC CC CC 8B C2", Utils::SigScan::PatternType::Address, void*)
		AddOffset(PlayerGetInventoryMoney, "gamedll_ph_x64_rwdi.dll", "8B C2 48 8B 44 C1", Utils::SigScan::PatternType::Address, void*)
		AddOffset(HandleInventoryItemsAmount, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 8B 29", Utils::SigScan::PatternType::Address, void*)
		AddOffset(SetNewWaypointLocation, "gamedll_ph_x64_rwdi.dll", "85 D2 78 ?? 48 89 74 24 ?? 57 48 83 EC ?? 49 8B F8", Utils::SigScan::PatternType::Address, void*)
		AddOffset(SetNewWaypointLocationWaypointIsSetBoolInstr, "gamedll_ph_x64_rwdi.dll", "C6 84 33 [?? ?? ?? ?? 01 48 8B 5C 24", Utils::SigScan::PatternType::Address, DWORD*)
		AddOffset(HandleHeadBob, "gamedll_ph_x64_rwdi.dll", "40 53 48 83 EC ?? 0F 29 74 24 ?? 48 8D 51 ?? 0F 29 7C 24 ?? 0F 28 F3 0F 28 FA F3 0F 11 74 24 ?? F3 0F 10 51 ?? 0F 28 D9 44 0F 29 44 24 ?? 48 8B D9 F3 0F 11 7C 24 ?? 44 0F 28 C1 E8 ?? ?? ?? ?? F3 0F 10 53 ?? 48 8D 53 ?? F3 0F 11 74 24 ?? 48 8D 4B ?? 41 0F 28 D8 F3 0F 11 7C 24 ?? E8 ?? ?? ?? ?? F3 0F 10 53 ?? 48 8D 53 ?? F3 0F 11 74 24 ?? 48 8D 4B ?? 41 0F 28 D8 F3 0F 11 7C 24 ?? E8 ?? ?? ?? ?? F3 0F 10 53", Utils::SigScan::PatternType::Address, void*)
		AddOffset(SomeFloatCalcFunc, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 57 48 81 EC ?? ?? ?? ?? F3 0F 10 0D", Utils::SigScan::PatternType::Address, void*)
		//AddOffset(CompareAndUpdateFloat, "gamedll_ph_x64_rwdi.dll", "0F 2F C1 73 ?? 0F 28 C1 C3", Utils::SigScan::PatternType::Address, void*)
		//AddOffset(HandlePlayerImmunity, "gamedll_ph_x64_rwdi.dll", "48 8B C4 53 56 57 41 56 41 57", Utils::SigScan::PatternType::Address, void*)
		//AddOffset(HandlePlayerImmunity2, "gamedll_ph_x64_rwdi.dll", "40 55 56 41 56 41 57 48 8D 6C 24 ?? 48 81 EC ?? ?? ?? ?? 48 8B F1 45 0F B6 F0", Utils::SigScan::PatternType::Address, void*)
		//AddOffset(HandleFallHeight, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 57 48 83 EC ?? 0F B6 FA 48 8B D9 0F B6 91", Utils::SigScan::PatternType::Address, void*)
		//AddOffset(HandlePlayerFall, "gamedll_ph_x64_rwdi.dll", "48 89 5C 24 ?? 57 48 83 EC ?? 0F 29 74 24 ?? 48 8B FA 0F 28 F2", Utils::SigScan::PatternType::Address, void*)
		//AddOffset(GetTimeWeatherSystem, "engine_x64_rwdi.dll", "E8 [?? ?? ?? ?? 33 D2 48 8B C8 E8 ?? ?? ?? ?? 49 8D 4F 38", PatternType::RelativePointer, void*)
		//AddOffset(SetForcedWeather, "engine_x64_rwdi.dll", "89 51 68 C3 CC CC CC CC CC CC CC CC CC CC CC CC", PatternType::Address, void*)
		//AddOffset(GetCurrentWeather, "engine_x64_rwdi.dll", "48 8B 41 78 48 85 C0 75 0F", PatternType::Address, void*)
		//AddOffset(GetForwardVector, "engine_x64_rwdi.dll", "4C 8B 41 38 41 8B 40 48", PatternType::Address, void*)
		//AddOffset(IsLoading, "engine_x64_rwdi.dll", "48 8B 05 ?? ?? ?? ?? 48 8B 51 38", PatternType::Address, void*)
		//AddOffset(ShowUIManager, "engine_x64_rwdi.dll", "48 8B 0D ?? ?? ?? ?? E9 ?? ?? ?? ?? CC CC CC CC 48 8B 49 ?? E9 ?? ?? ?? ?? CC CC CC CC CC CC CC 48 8B 49 ?? E9 ?? ?? ?? ?? CC CC CC CC CC CC CC 48 8B 49 ?? E9 ?? ?? ?? ?? CC CC CC CC CC CC CC 48 8B 49 ?? E9 ?? ?? ?? ?? CC CC CC CC CC CC CC 40 53", PatternType::Address, void*)
		//AddOffset(GetGameTimeDelta, "engine_x64_rwdi.dll", "E8 [?? ?? ?? ?? F3 0F 59 05 ?? ?? ?? ?? F3 0F 58 03", PatternType::RelativePointer, void*)
	};

#undef AddOffset
#undef AddStaticOffset
#undef AddStaticOffsetFromModule
#undef AddVTOffset
}