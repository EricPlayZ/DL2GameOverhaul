#include <EGSDK\Engine\IBaseCamera.h>
#include <EGSDK\Utils\Memory.h>
#include <EGSDK\Utils\WinMemory.h>

namespace EGSDK::Engine {
	bool IBaseCamera::isSetFOVCalledByEGSDK = false;

	float IBaseCamera::GetFOV() {
		return Utils::Memory::SafeCallFunction<float>("engine_x64_rwdi.dll", "?GetFOV@IBaseCamera@@QEBAMXZ", -1.0f, this);
	}
	Vec3* IBaseCamera::GetForwardVector(Vec3* outForwardVec) {
		return Utils::Memory::SafeCallFunction<Vec3*>("engine_x64_rwdi.dll", "?GetForwardVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outForwardVec);
	}
	Vec3* IBaseCamera::GetUpVector(Vec3* outUpVec) {
		return Utils::Memory::SafeCallFunction<Vec3*>("engine_x64_rwdi.dll", "?GetUpVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outUpVec);
	}
	Vec3* IBaseCamera::GetLeftVector(Vec3* outLeftVec) {
		return Utils::Memory::SafeCallFunction<Vec3*>("engine_x64_rwdi.dll", "?GetLeftVector@IBaseCamera@@QEBA?BVvec3@@XZ", nullptr, this, outLeftVec);
	}
	Vec3* IBaseCamera::GetPosition(Vec3* outPos) {
		return Utils::Memory::SafeCallFunction<Vec3*>("engine_x64_rwdi.dll", "?GetPosition@IBaseCamera@@UEBA?BVvec3@@XZ", nullptr, this, outPos);
	}
	Mtx34* IBaseCamera::GetViewMatrix() {
		return Utils::Memory::SafeCallFunction<Mtx34*>("engine_x64_rwdi.dll", "?GetViewMatrix@IBaseCamera@@QEAAAEBVmtx34@@XZ", nullptr, this);
	}
	Mtx34* IBaseCamera::GetInvCameraMatrix() {
		return Utils::Memory::SafeCallFunction<Mtx34*>("engine_x64_rwdi.dll", "?GetInvCameraMatrix@IBaseCamera@@QEAAAEBVmtx34@@XZ", nullptr, this);
	}

	void IBaseCamera::Rotate(float angle, const Vec3* axis) {
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?Rotate@IBaseCamera@@QEAAXMAEBVvec3@@@Z", this, angle, axis);
	}
	void IBaseCamera::SetFOV(float fov) {
		isSetFOVCalledByEGSDK = true;
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetFOV@IBaseCamera@@QEAAXM@Z", this, fov);
		isSetFOVCalledByEGSDK = false;
	}
	void IBaseCamera::SetPosition(const Vec3* pos) {
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetPosition@IBaseCamera@@QEAAXAEBVvec3@@@Z", this, pos);
	}
	void IBaseCamera::SetCameraMatrix(const Mtx34* mtx) {
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetCameraMatrix@IBaseCamera@@QEAAXAEBVmtx34@@@Z", this, mtx);
	}
	void IBaseCamera::SetInvCameraMatrix(const Mtx34* mtx) {
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetInvCameraMatrix@IBaseCamera@@QEAAXAEBVmtx34@@@Z", this, mtx);
	}
}