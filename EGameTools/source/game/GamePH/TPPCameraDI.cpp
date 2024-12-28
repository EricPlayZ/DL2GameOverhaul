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
	SafeGetterDepCustomVT(TPPCameraDI, GetOffset_TPPCameraDI, false, "gamedll_ph_x64_rwdi.dll")
}