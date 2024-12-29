#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class EGameSDK_API ItemDescWithContext {
	public:
		union {
			buffer<0xA8, float> weaponDurability;
		};
	};
}