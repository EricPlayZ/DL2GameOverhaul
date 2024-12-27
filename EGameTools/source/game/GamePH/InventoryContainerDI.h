#pragma once
#include "..\ClassHelpers.h"
#include "InventoryMoney.h"

namespace GamePH {
	class InventoryContainerDI {
	public:
		InventoryMoney* GetInventoryMoney(UINT indexMaybe);
	};
}