#include <pch.h>
#include "..\offsets.h"
#include "CBulletPhysicsCharacter.h"
#include "CoPhysicsProperty.h"

namespace Engine {
	Vector3 CBulletPhysicsCharacter::posBeforeFreeze{};

	void CBulletPhysicsCharacter::FreezeCharacter() {
		MoveCharacter(posBeforeFreeze);
	}
	void CBulletPhysicsCharacter::MoveCharacter(const Vector3& pos) {
		playerDownwardVelocity = 0.0f;
		playerPos = pos;
		playerPos2 = pos;
	}

	SafeGetterDepVT(CBulletPhysicsCharacter, CoPhysicsProperty, "engine_x64_rwdi.dll")
}