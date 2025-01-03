#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\CoBaseCameraProxy.h>
#include <EGSDK\GamePH\FreeCamera.h>
#include <EGSDK\GamePH\TPPCameraDI.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static TPPCameraDI* GetOffset_TPPCameraDI() {
		FreeCamera* pFreeCam = FreeCamera::Get();
		if (!pFreeCam)
			return nullptr;

		CoBaseCameraProxy* pCoBaseCameraProxy = pFreeCam->pCoBaseCameraProxy;
		if (!pCoBaseCameraProxy)
			return nullptr;

		return pCoBaseCameraProxy->pTPPCameraDI;
	}
	TPPCameraDI* TPPCameraDI::Get() {
		return ClassHelpers::SafeGetter<TPPCameraDI>(GetOffset_TPPCameraDI, false);
	}
}