#pragma once
#include "..\ClassHelpers.h"

namespace GamePH {
	class PlayerVariables;

	class PlayerState {
	public:
		union {
			buffer<0x300, PlayerVariables*> pPlayerVariables;
		};

		static PlayerState* Get();
	};
}