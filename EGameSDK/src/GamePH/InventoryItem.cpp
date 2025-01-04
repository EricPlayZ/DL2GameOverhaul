#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\InventoryItem.h>
#include <EGSDK\GamePH\ItemDescWithContext.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static ItemDescWithContext* GetOffset_ItemDescWithContext(InventoryItem* pInventoryItem) {
		return reinterpret_cast<ItemDescWithContext*>(reinterpret_cast<DWORD64>(pInventoryItem) + 0x40);
	}
	ItemDescWithContext* InventoryItem::GetItemDescCtx() {
		return ClassHelpers::SafeGetter<ItemDescWithContext>(GetOffset_ItemDescWithContext, false, true, this);
	}
}