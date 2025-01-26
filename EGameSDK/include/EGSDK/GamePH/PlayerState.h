#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class PlayerVariables;

	class EGameSDK_API PlayerState {
	public:
		union {
			DynamicField(PlayerState, PlayerVariables*, playerVariables);
		};

		static PlayerState* Get();
	};
}