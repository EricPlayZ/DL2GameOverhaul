#pragma once
#include <stdint.h>
#include <EGSDK\Mtx34.h>

namespace EGSDK::Engine {
	struct EGameSDK_API CameraMtx {
		Mtx34* mtx1{};
		Mtx34* mtx2{};
	};

	class EGameSDK_API IBaseCamera {
	public:
		float GetFOV();
		Vec3* GetForwardVector(Vec3* outForwardVec);
		Vec3* GetUpVector(Vec3* outUpVec);
		Vec3* GetLeftVector(Vec3* outLeftVec);
		Vec3* GetPosition(Vec3* outPos);
		CameraMtx* GetViewMatrix();

		void SetFOV(float fov);
		void SetPosition(const Vec3* pos);
		void SetCameraMatrix(const CameraMtx* mtx);

		static bool isSetFOVCalledByEGSDK;
	};
}