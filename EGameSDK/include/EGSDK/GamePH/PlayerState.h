#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class PlayerVariables;

	class EGameSDK_API PlayerState {
	public:
		union {
			ClassHelpers::buffer<0x300, PlayerVariables*> playerVariables;
		};

		static PlayerState* Get();
	};
}