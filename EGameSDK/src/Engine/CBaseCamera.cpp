#include <Windows.h>
#include <EGSDK\Engine\CBaseCamera.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Engine {
	bool CBaseCamera::isSetFOVCalledByEGT = false;

	float CBaseCamera::GetFOV() {
		return Utils::Memory::SafeCallFunction<float>("engine_x64_rwdi.dll", "?GetFOV@IBaseCamera@@QEBAMXZ", -1.0f, this);
	}
	Vector3* CBaseCamera::GetForwardVector(Vector3* outForwardVec) {
		return Utils::Memory::SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetForwardVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outForwardVec);
	}
	Vector3* CBaseCamera::GetUpVector(Vector3* outUpVec) {
		return Utils::Memory::SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetUpVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outUpVec);
	}
	Vector3* CBaseCamera::GetLeftVector(Vector3* outLeftVec) {
		return Utils::Memory::SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetLeftVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outLeftVec);
	}
	Vector3* CBaseCamera::GetPosition(Vector3* outPos) {
		return Utils::Memory::SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetPosition@IBaseCamera@@UEBA?BVvec3@@XZ", nullptr, this, outPos);
	}

	void CBaseCamera::SetFOV(float fov) {
		isSetFOVCalledByEGT = true;
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetFOV@IBaseCamera@@QEAAXM@Z", this, fov);
		isSetFOVCalledByEGT = false;
	}
	void CBaseCamera::SetPosition(const Vector3* pos) {
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetPosition@IBaseCamera@@QEAAXAEBVvec3@@@Z", this, pos);
	}
}