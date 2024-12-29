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
		return _SafeGetter<TPPCameraDI>(GetOffset_TPPCameraDI, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_TPPCameraDI);
	}
}