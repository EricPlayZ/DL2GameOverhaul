#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\InventoryContainerDI.h>
#include <EGSDK\GamePH\InventoryMoney.h>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\WinMemory.h>

namespace EGSDK::GamePH {
	static InventoryMoney* GetOffset_InventoryMoney(InventoryContainerDI* pInventoryContainerDI, uint32_t indexMaybe) {
		return Utils::Memory::SafeCallFunctionOffset<InventoryMoney*>(OffsetManager::Get_PlayerGetInventoryMoney, nullptr, pInventoryContainerDI, indexMaybe);
	}
	InventoryMoney* InventoryContainerDI::GetInventoryMoney(UINT indexMaybe) {
		return ClassHelpers::SafeGetter<InventoryMoney>(GetOffset_InventoryMoney, false, true, this, indexMaybe);
	}
}