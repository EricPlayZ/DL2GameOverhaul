#pragma once
#include <basetsd.h>
#include "..\ClassHelpers.h"

namespace GamePH {
	class SessionCooperativeDI;

	class GameDI_PH {
	public:
		union {
			buffer<0x130, SessionCooperativeDI*> pSessionCooperativeDI;
			buffer<0x910, bool> blockPauseGameOnPlayerAfk;
		};

		float GetGameTimeDelta();
		void TogglePhotoMode(bool doNothing = false, bool setAsOptionalCamera = false);

		static GameDI_PH* Get();
	};
}