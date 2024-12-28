#include <pch.h>
#include "..\offsets.h"
#include "InventoryContainerDI.h"
#include "InventoryItem.h"
#include "LocalClientDI.h"
#include "PlayerDI_PH.h"

namespace GamePH {
	static InventoryItem* GetOffset_CurrentWeapon(PlayerDI_PH* pPlayerDI_PH, UINT indexMaybe) {
		return _SafeCallFunctionOffset<InventoryItem*>(Offsets::Get_PlayerGetCurrentWeapon, nullptr, pPlayerDI_PH, indexMaybe);
	}
	InventoryItem* PlayerDI_PH::GetCurrentWeapon(UINT indexMaybe) {
		return _SafeGetter<InventoryItem>(GetOffset_CurrentWeapon, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_InventoryItem, this, indexMaybe);
	}

	static InventoryContainerDI* GetOffset_InventoryContainer(PlayerDI_PH* pPlayerDI_PH) {
		return reinterpret_cast<InventoryContainerDI*>(*reinterpret_cast<DWORD64*>(reinterpret_cast<DWORD64>(pPlayerDI_PH) + 0x470));
	}
	InventoryContainerDI* PlayerDI_PH::GetInventoryContainer() {
		return _SafeGetter<InventoryContainerDI>(GetOffset_InventoryContainer, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_InventoryContainerDI, this);
	}

	SafeGetterDepVT(PlayerDI_PH, LocalClientDI, "gamedll_ph_x64_rwdi.dll")
}