#pragma once
#include <EGSDK\Vec3.h>
#include <EGSDK\Engine\IPhysicsCharacter.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	class EGameSDK_API CBulletPhysicsCharacter : IPhysicsCharacter {
	public:
		union {
			DynamicField(CBulletPhysicsCharacter, Vec3, playerPos);
			DynamicField(CBulletPhysicsCharacter, float, playerDownwardVelocity);
		};

		static Vec3 posBeforeFreeze;

		void FreezeCharacter();
		void MoveCharacter(const Vec3& pos);

		static CBulletPhysicsCharacter* Get();
	};
}