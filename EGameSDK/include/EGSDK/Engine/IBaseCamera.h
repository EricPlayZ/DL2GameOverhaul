#pragma once
#include <EGSDK\Vector3.h>

namespace EGSDK::Engine {
	class EGameSDK_API IBaseCamera {
	public:
		float GetFOV();
		Vector3* GetForwardVector(Vector3* outForwardVec);
		Vector3* GetUpVector(Vector3* outUpVec);
		Vector3* GetLeftVector(Vector3* outLeftVec);
		Vector3* GetPosition(Vector3* outPos);

		void SetFOV(float fov);
		void SetPosition(const Vector3* pos);

		static bool isSetFOVCalledByEGSDK;
	};
}