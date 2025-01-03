#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK {
	namespace GamePH {
		class GameDI_PH;
	}

	namespace Engine {
		class CVideoSettings;
		class CLevel;

		class EGameSDK_API CGame {
		public:
			union {
				ClassHelpers::buffer<0x8, GamePH::GameDI_PH*> pGameDI_PH;
				ClassHelpers::buffer<0x28, CVideoSettings*> pCVideoSettings;
				ClassHelpers::buffer<0x390, CLevel*> pCLevel;
			};

			static CGame* Get();
		};
	}
}