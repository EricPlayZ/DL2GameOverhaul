#include <pch.h>
#include "..\offsets.h"
#include "InventoryItem.h"
#include "ItemDescWithContext.h"

namespace GamePH {
	static ItemDescWithContext* GetOffset_ItemDescWithContext(InventoryItem* pInventoryItem) {
		return reinterpret_cast<ItemDescWithContext*>(reinterpret_cast<DWORD64>(pInventoryItem) + 0x40);
	}
	ItemDescWithContext* InventoryItem::GetItemDescCtx() {
		return _SafeGetter<ItemDescWithContext>(GetOffset_ItemDescWithContext, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_ItemDescWithContext, this);
	}
}