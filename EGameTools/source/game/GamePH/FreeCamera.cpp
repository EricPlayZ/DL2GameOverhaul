#include <pch.h>
#include "..\offsets.h"
#include "FreeCamera.h"

namespace GamePH {
	void FreeCamera::AllowCameraMovement(int mode) {
		_SafeCallFunctionOffsetVoid(Offsets::Get_AllowCameraMovement, this, mode);
	}

	SafeGetterVT(FreeCamera, "gamedll_ph_x64_rwdi.dll")
}