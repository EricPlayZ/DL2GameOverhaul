#pragma once
#include "..\ClassHelpers.h"

namespace GamePH {
	class PlayerVariables;

	class PlayerState {
	public:
		union {
			buffer<0x300, PlayerVariables*> playerVars;
		};

		static PlayerState* Get();
	};
}