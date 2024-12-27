#pragma once
#include "..\ClassHelpers.h"

namespace Engine {
	class CBulletPhysicsCharacter;

	class CoPhysicsProperty {
	public:
		union {
			buffer<0x20, CBulletPhysicsCharacter*> pCBulletPhysicsCharacter;
		};

		static CoPhysicsProperty* Get();
	};
}