#pragma once
#include "..\ClassHelpers.h"

namespace GamePH {
	class ItemDescWithContext {
	public:
		union {
			buffer<0xA8, float> weaponDurability;
		};
	};
}