#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\FreeCamera.h>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\WinMemory.h>

namespace EGSDK::GamePH {
	void FreeCamera::AllowCameraMovement(int mode) {
		Utils::Memory::SafeCallFunctionOffsetVoid(OffsetManager::Get_AllowCameraMovement, this, mode);
	}

	FreeCamera* FreeCamera::Get() {
		return ClassHelpers::SafeGetter<FreeCamera>(OffsetManager::Get_FreeCamera, true);
	}
}