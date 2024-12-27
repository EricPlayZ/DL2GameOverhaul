#pragma once
#include "..\ClassHelpers.h"
#include "ItemDescWithContext.h"

namespace GamePH {
	class InventoryItem {
	public:
		ItemDescWithContext* GetItemDescCtx();
	};
}