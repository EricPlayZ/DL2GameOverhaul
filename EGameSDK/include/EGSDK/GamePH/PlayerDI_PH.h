#pragma once
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\GamePH\InventoryItem.h>
#include <EGSDK\GamePH\InventoryContainerDI.h>

namespace EGSDK::Engine {
	class CoPhysicsProperty;
}

namespace EGSDK::GamePH {
	class EGameSDK_API PlayerDI_PH {
	public:
		union {
			ClassHelpers::buffer<0xF0, Engine::CoPhysicsProperty*> pCoPhysicsProperty;
			ClassHelpers::buffer<0x3520, bool> restrictionsEnabled;
			ClassHelpers::buffer<0x35E9, bool> enableTPPModel1;
			ClassHelpers::buffer<0x35EA, bool> enableTPPModel2;
		};

		static bool areRestrictionsEnabledByGame;

		InventoryItem* GetCurrentWeapon(UINT indexMaybe);
		InventoryContainerDI* GetInventoryContainer();

		bool EnablePlayerRestrictions(DWORD64* flags);
		bool DisablePlayerRestrictions(DWORD64* flags);
		bool HandlePlayerRestrictions();

		static PlayerDI_PH* Get();
	};
}