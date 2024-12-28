#include <pch.h>
#include "..\offsets.h"
#include "InventoryContainerDI.h"
#include "InventoryMoney.h"

namespace GamePH {
	static InventoryMoney* GetOffset_InventoryMoney(InventoryContainerDI* pInventoryContainerDI, UINT indexMaybe) {
		return _SafeCallFunctionOffset<InventoryMoney*>(Offsets::Get_PlayerGetInventoryMoney, nullptr, pInventoryContainerDI, indexMaybe);
	}
	InventoryMoney* InventoryContainerDI::GetInventoryMoney(UINT indexMaybe) {
		return _SafeGetter<InventoryMoney>(GetOffset_InventoryMoney, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_InventoryContainerDI, this, indexMaybe);
	}
}