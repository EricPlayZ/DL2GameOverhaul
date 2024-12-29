#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class PlayerVariables;

	class EGameSDK_API PlayerState {
	public:
		union {
			buffer<0x300, PlayerVariables*> pPlayerVariables;
		};

		static PlayerState* Get();
	};
}