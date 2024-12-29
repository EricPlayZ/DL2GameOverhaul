#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class PlayerDI_PH;

	class EGameSDK_API PlayerInfectionModule {
	public:
		union {
			buffer<0x8, PlayerDI_PH*> pPlayerDI_PH;
			buffer<0x20, float> maxImmunity;
			buffer<0x2C, float> immunity;
		};
		
		static std::vector<PlayerInfectionModule*> playerInfectionModulePtrList;

		static PlayerInfectionModule* Get();
		static void Set(void* instance);

		static void UpdateClassAddr();
	};
}