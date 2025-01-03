#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	class CBulletPhysicsCharacter;

	class EGameSDK_API CoPhysicsProperty {
	public:
		union {
			ClassHelpers::buffer<0x20, CBulletPhysicsCharacter*> pCBulletPhysicsCharacter;
		};

		static CoPhysicsProperty* Get();
	};
}