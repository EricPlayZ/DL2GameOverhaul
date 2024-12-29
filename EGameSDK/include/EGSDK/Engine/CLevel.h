#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK {
	namespace GamePH {
		class LevelDI;
	}

	namespace Engine {
		class CGSObject;

		class EGameSDK_API CLevel {
		public:
			union {
				buffer<0x20, GamePH::LevelDI*> pLevelDI;
				buffer<0x30, CGSObject*> pCGSObject;
			};

			static CLevel* Get();
		};
	}
}