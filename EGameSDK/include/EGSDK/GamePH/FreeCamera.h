#pragma once
#include <EGSDK\Engine\CBaseCamera.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class CoBaseCameraProxy;

	class EGameSDK_API FreeCamera : public Engine::CBaseCamera {
	public:
		union {
			ClassHelpers::buffer<0x18, CoBaseCameraProxy*> pCoBaseCameraProxy;
			ClassHelpers::buffer<0x38, Engine::CBaseCamera*> pCBaseCamera;
			ClassHelpers::buffer<0x42, bool> enableSpeedMultiplier1;
			ClassHelpers::buffer<0x43, bool> enableSpeedMultiplier2;
			ClassHelpers::buffer<0x1B4, float> FOV;
			ClassHelpers::buffer<0x1CC, float> speedMultiplier;
		};

		void AllowCameraMovement(int mode = 2);

		static FreeCamera* Get();
	};
}