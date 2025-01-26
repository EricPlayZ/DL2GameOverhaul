#pragma once
#include <stdint.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class EGameSDK_API CoPlayerRestrictions {
	public:
		union {
			DynamicField(CoPlayerRestrictions, uint64_t, flags);
		};
		uint64_t* GetPlayerRestrictionsFlags(uint64_t* flagsOut);

		static CoPlayerRestrictions* Get();
		static void SetInstance(void* instance);
	};
}