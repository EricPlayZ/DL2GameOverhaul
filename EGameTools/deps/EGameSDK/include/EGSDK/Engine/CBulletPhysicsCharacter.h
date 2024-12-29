#pragma once
#include <EGSDK\Vector3.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	class EGameSDK_API CBulletPhysicsCharacter {
	public:
		union {
			buffer<0xCB8, Vector3> playerPos2;
			buffer<0xCD0, Vector3> playerPos;
			buffer<0x1050, float> playerDownwardVelocity;
		};

		static Vector3 posBeforeFreeze;

		void FreezeCharacter();
		void MoveCharacter(const Vector3& pos);

		static CBulletPhysicsCharacter* Get();
	};
}