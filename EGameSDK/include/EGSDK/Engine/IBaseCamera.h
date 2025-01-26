#pragma once
#include <stdint.h>
#include <EGSDK\Mtx34.h>

namespace EGSDK::Engine {
	class EGameSDK_API IBaseCamera {
	public:
		float GetFOV();
		Vec3* GetForwardVector(Vec3* outForwardVec);
		Vec3* GetUpVector(Vec3* outUpVec);
		Vec3* GetLeftVector(Vec3* outLeftVec);
		Vec3* GetPosition(Vec3* outPos);
		Mtx34* GetViewMatrix();
		Mtx34* GetInvCameraMatrix();

		void Rotate(float angle, const Vec3* axis);
		void SetFOV(float fov);
		void SetPosition(const Vec3* pos);
		void SetCameraMatrix(const Mtx34* mtx);
		void SetInvCameraMatrix(const Mtx34* mtx);

		static bool isSetFOVCalledByEGSDK;
	};
}