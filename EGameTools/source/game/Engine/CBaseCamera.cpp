#include <pch.h>
#include "CBaseCamera.h"

namespace Engine {
	Vector3* CBaseCamera::GetForwardVector(Vector3* outForwardVec) {
		return _SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetForwardVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outForwardVec);
	}
	Vector3* CBaseCamera::GetUpVector(Vector3* outUpVec) {
		return _SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetUpVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outUpVec);
	}
	Vector3* CBaseCamera::GetLeftVector(Vector3* outLeftVec) {
		return _SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetLeftVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outLeftVec);
	}
	Vector3* CBaseCamera::GetPosition(Vector3* outPos) {
		return _SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetPosition@IBaseCamera@@UEBA?BVvec3@@XZ", nullptr, this, outPos);
	}

	void CBaseCamera::SetPosition(const Vector3* pos) {
		_SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetPosition@IBaseCamera@@QEAAXAEBVvec3@@@Z", this, pos);
	}
	void CBaseCamera::SetFOV(float fov) {
		_SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetFOV@IBaseCamera@@QEAAXM@Z", this, fov);
	}
}