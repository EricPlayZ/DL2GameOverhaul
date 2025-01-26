#include <EGSDK\Engine\IBaseCamera.h>
#include <EGSDK\Utils\Memory.h>
#include <EGSDK\Utils\WinMemory.h>

namespace EGSDK::Engine {
	bool IBaseCamera::isSetFOVCalledByEGSDK = false;

	float IBaseCamera::GetFOV() {
		return Utils::Memory::SafeCallFunction<float>("engine_x64_rwdi.dll", "?GetFOV@IBaseCamera@@QEBAMXZ", -1.0f, this);
	}
	Vector3* IBaseCamera::GetForwardVector(Vector3* outForwardVec) {
		return Utils::Memory::SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetForwardVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outForwardVec);
	}
	Vector3* IBaseCamera::GetUpVector(Vector3* outUpVec) {
		return Utils::Memory::SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetUpVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outUpVec);
	}
	Vector3* IBaseCamera::GetLeftVector(Vector3* outLeftVec) {
		return Utils::Memory::SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetLeftVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outLeftVec);
	}
	Vector3* IBaseCamera::GetPosition(Vector3* outPos) {
		return Utils::Memory::SafeCallFunction<Vector3*>("engine_x64_rwdi.dll", "?GetPosition@IBaseCamera@@UEBA?BVvec3@@XZ", nullptr, this, outPos);
	}

	void IBaseCamera::SetFOV(float fov) {
		isSetFOVCalledByEGSDK = true;
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetFOV@IBaseCamera@@QEAAXM@Z", this, fov);
		isSetFOVCalledByEGSDK = false;
	}
	void IBaseCamera::SetPosition(const Vector3* pos) {
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetPosition@IBaseCamera@@QEAAXAEBVvec3@@@Z", this, pos);
	}
}