#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <EGSDK\Engine\CBaseCamera.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Engine {
	Vector3* CBaseCamera::GetForwardVector(Vector3* outForwardVec) {
		return Utils::Memory::_SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetForwardVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outForwardVec);
	}
	Vector3* CBaseCamera::GetUpVector(Vector3* outUpVec) {
		return Utils::Memory::_SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetUpVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outUpVec);
	}
	Vector3* CBaseCamera::GetLeftVector(Vector3* outLeftVec) {
		return Utils::Memory::_SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetLeftVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outLeftVec);
	}
	Vector3* CBaseCamera::GetPosition(Vector3* outPos) {
		return Utils::Memory::_SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetPosition@IBaseCamera@@UEBA?BVvec3@@XZ", nullptr, this, outPos);
	}

	void CBaseCamera::SetPosition(const Vector3* pos) {
		Utils::Memory::_SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetPosition@IBaseCamera@@QEAAXAEBVvec3@@@Z", this, pos);
	}
	void CBaseCamera::SetFOV(float fov) {
		Utils::Memory::_SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetFOV@IBaseCamera@@QEAAXM@Z", this, fov);
	}
}