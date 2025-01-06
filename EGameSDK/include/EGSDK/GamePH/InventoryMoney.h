#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class EGameSDK_API InventoryMoney {
	public:
		union {
			ClassHelpers::buffer<0x38, int> oldWorldMoney;
		};
	};
}