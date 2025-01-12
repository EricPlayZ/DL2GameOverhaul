#pragma once
#include <EGSDK\Vector3.h>

namespace EGSDK::Engine {
	class EGameSDK_API CBaseCamera {
	public:
		/*union {
			ClassHelpers::buffer<0x48, float> yaw;
			ClassHelpers::buffer<0x4C, float> X;
			ClassHelpers::buffer<0x58, float> pitch;
			ClassHelpers::buffer<0x5C, float> Y;
			ClassHelpers::buffer<0x6C, float> Z;
		};*/
		float GetFOV();
		Vector3* GetForwardVector(Vector3* outForwardVec);
		Vector3* GetUpVector(Vector3* outUpVec);
		Vector3* GetLeftVector(Vector3* outLeftVec);
		Vector3* GetPosition(Vector3* outPos);

		void SetFOV(float fov);
		void SetPosition(const Vector3* pos);

		static bool isSetFOVCalledByEGT;
	};
}