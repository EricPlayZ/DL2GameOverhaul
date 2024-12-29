#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class SessionCooperativeDI;

	class EGameSDK_API GameDI_PH {
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