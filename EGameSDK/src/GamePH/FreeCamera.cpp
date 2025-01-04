#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\FreeCamera.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	void FreeCamera::AllowCameraMovement(int mode) {
		Utils::Memory::SafeCallFunctionOffsetVoid(Offsets::Get_AllowCameraMovement, this, mode);
	}

	FreeCamera* FreeCamera::Get() {
		return ClassHelpers::SafeGetter<FreeCamera>(Offsets::Get_FreeCamera, true);
	}
}