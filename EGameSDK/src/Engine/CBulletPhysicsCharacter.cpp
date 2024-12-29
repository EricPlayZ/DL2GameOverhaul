#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CBulletPhysicsCharacter.h>
#include <EGSDK\Engine\CoPhysicsProperty.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	Vector3 CBulletPhysicsCharacter::posBeforeFreeze{};

	void CBulletPhysicsCharacter::FreezeCharacter() {
		MoveCharacter(posBeforeFreeze);
	}
	void CBulletPhysicsCharacter::MoveCharacter(const Vector3& pos) {
		playerDownwardVelocity = 0.0f;
		playerPos = pos;
		playerPos2 = pos;
	}

	static CBulletPhysicsCharacter* GetOffset_CBulletPhysicsCharacter() {
		CoPhysicsProperty* pCoPhysicsProperty = CoPhysicsProperty::Get();
		return pCoPhysicsProperty ? pCoPhysicsProperty->pCBulletPhysicsCharacter : nullptr;
	}
	CBulletPhysicsCharacter* CBulletPhysicsCharacter::Get() {
		return _SafeGetter<CBulletPhysicsCharacter>(GetOffset_CBulletPhysicsCharacter, "engine_x64_rwdi.dll", false, Offsets::GetVT_CBulletPhysicsCharacter);
	}
}