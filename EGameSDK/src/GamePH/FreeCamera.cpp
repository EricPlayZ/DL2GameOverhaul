#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\FreeCamera.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	void FreeCamera::AllowCameraMovement(int mode) {
		Utils::Memory::_SafeCallFunctionOffsetVoid(Offsets::Get_AllowCameraMovement, this, mode);
	}

	FreeCamera* FreeCamera::Get() {
		return _SafeGetter<FreeCamera>(Offsets::Get_FreeCamera, "gamedll_ph_x64_rwdi.dll", true, Offsets::GetVT_FreeCamera);
	}
}