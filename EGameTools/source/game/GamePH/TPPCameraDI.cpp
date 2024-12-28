#include <pch.h>
#include "..\offsets.h"
#include "CoBaseCameraProxy.h"
#include "FreeCamera.h"
#include "TPPCameraDI.h"

namespace GamePH {
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