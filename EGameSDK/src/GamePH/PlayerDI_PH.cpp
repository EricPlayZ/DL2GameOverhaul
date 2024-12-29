#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\InventoryContainerDI.h>
#include <EGSDK\GamePH\InventoryItem.h>
#include <EGSDK\GamePH\LocalClientDI.h>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static InventoryItem* GetOffset_CurrentWeapon(PlayerDI_PH* pPlayerDI_PH, UINT indexMaybe) {
		return Utils::Memory::_SafeCallFunctionOffset<InventoryItem*>(Offsets::Get_PlayerGetCurrentWeapon, nullptr, pPlayerDI_PH, indexMaybe);
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

	static PlayerDI_PH* GetOffset_PlayerDI_PH() {
		LocalClientDI* pLocalClientDI = LocalClientDI::Get();
		return pLocalClientDI ? pLocalClientDI->pPlayerDI_PH : nullptr;
	}
	PlayerDI_PH* PlayerDI_PH::Get() {
		return _SafeGetter<PlayerDI_PH>(GetOffset_PlayerDI_PH, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_PlayerDI_PH);
	}
}