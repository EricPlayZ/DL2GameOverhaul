#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class EGameSDK_API CoPlayerRestrictions {
	public:
		union {
			ClassHelpers::buffer<0x1F0, DWORD64> flags;
		};
		DWORD64* GetPlayerRestrictionsFlags(DWORD64* flagsOut);

		static CoPlayerRestrictions* Get();
		static void SetInstance(void* instance);
	};
}