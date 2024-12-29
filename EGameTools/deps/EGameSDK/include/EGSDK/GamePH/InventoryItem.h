#pragma once
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\GamePH\ItemDescWithContext.h>

namespace EGSDK::GamePH {
	class EGameSDK_API InventoryItem {
	public:
		ItemDescWithContext* GetItemDescCtx();
	};
}