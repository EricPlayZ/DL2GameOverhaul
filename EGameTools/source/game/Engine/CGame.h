#pragma once
#include "..\ClassHelpers.h"

namespace GamePH {
	class GameDI_PH;
}

namespace Engine {
	class CVideoSettings;
	class CLevel;

	class CGame {
	public:
		union {
			buffer<0x8, GamePH::GameDI_PH*> pGameDI_PH;
			buffer<0x28, CVideoSettings*> pCVideoSettings;
			buffer<0x390, CLevel*> pCLevel;
		};

		static CGame* Get();
	};
}