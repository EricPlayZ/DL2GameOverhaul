#include <pch.h>
#include "..\offsets.h"
#include "FreeCamera.h"

namespace GamePH {
	void FreeCamera::AllowCameraMovement(int mode) {
		_SafeCallFunctionOffsetVoid(Offsets::Get_AllowCameraMovement, this, mode);
	}

	FreeCamera* FreeCamera::Get() {
		return _SafeGetter<FreeCamera>(Offsets::Get_FreeCamera, "gamedll_ph_x64_rwdi.dll", true, Offsets::GetVT_FreeCamera);
	}
}