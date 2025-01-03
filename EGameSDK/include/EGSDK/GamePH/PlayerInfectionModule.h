#pragma once
#include <vector>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class PlayerDI_PH;

	class EGameSDK_API PlayerInfectionModule {
	public:
		union {
			ClassHelpers::buffer<0x8, PlayerDI_PH*> pPlayerDI_PH;
			ClassHelpers::buffer<0x20, float> maxImmunity;
			ClassHelpers::buffer<0x2C, float> immunity;
		};
		
		~PlayerInfectionModule();
		static PlayerInfectionModule* Get();

		static void EmplaceBack(PlayerInfectionModule* ptr);
		static void UpdateClassAddr();
	private:
		static std::vector<PlayerInfectionModule*>* playerInfectionModulePtrList;

		static void SetInstance(void* instance);
	};
}