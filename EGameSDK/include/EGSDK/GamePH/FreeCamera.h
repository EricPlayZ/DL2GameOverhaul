#pragma once
#include <EGSDK\Engine\CBaseCamera.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class CoBaseCameraProxy;

	class EGameSDK_API FreeCamera : public Engine::CBaseCamera {
	public:
		union {
			buffer<0x18, CoBaseCameraProxy*> pCoBaseCameraProxy;
			buffer<0x38, Engine::CBaseCamera*> pCBaseCamera;
			buffer<0x42, bool> enableSpeedMultiplier1;
			buffer<0x43, bool> enableSpeedMultiplier2;
			buffer<0x1B4, float> FOV;
			buffer<0x1CC, float> speedMultiplier;
		};

		void AllowCameraMovement(int mode = 2);

		static FreeCamera* Get();
	};
}