#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\InventoryContainerDI.h>
#include <EGSDK\GamePH\InventoryMoney.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static InventoryMoney* GetOffset_InventoryMoney(InventoryContainerDI* pInventoryContainerDI, UINT indexMaybe) {
		return Utils::Memory::SafeCallFunctionOffset<InventoryMoney*>(Offsets::Get_PlayerGetInventoryMoney, nullptr, pInventoryContainerDI, indexMaybe);
	}
	InventoryMoney* InventoryContainerDI::GetInventoryMoney(UINT indexMaybe) {
		return ClassHelpers::SafeGetter<InventoryMoney>(GetOffset_InventoryMoney, false, true, this, indexMaybe);
	}
}