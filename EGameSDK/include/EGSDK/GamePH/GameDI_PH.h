#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class SessionCooperativeDI;

	class EGameSDK_API GameDI_PH {
	public:
		union {
			DynamicField(GameDI_PH, bool, blockPauseGameOnPlayerAfk);
			DynamicField(GameDI_PH, SessionCooperativeDI*, pSessionCooperativeDI);
		};

		float GetGameTimeDelta();
		void TogglePhotoMode(bool doNothing = false, bool setAsOptionalCamera = false);

		static GameDI_PH* Get();
	};
}