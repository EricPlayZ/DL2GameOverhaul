#pragma once
#include <EGSDK\Vector3.h>
#include <EGSDK\Engine\IPhysicsCharacter.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	class EGameSDK_API CBulletPhysicsCharacter : IPhysicsCharacter {
	public:
		union {
			DynamicField(CBulletPhysicsCharacter, Vector3, playerPos);
			DynamicField(CBulletPhysicsCharacter, Vector3, playerPos2);
			DynamicField(CBulletPhysicsCharacter, float, playerDownwardVelocity);
		};

		static Vector3 posBeforeFreeze;

		void FreezeCharacter();
		void MoveCharacter(const Vector3& pos);

		static CBulletPhysicsCharacter* Get();
	};
}