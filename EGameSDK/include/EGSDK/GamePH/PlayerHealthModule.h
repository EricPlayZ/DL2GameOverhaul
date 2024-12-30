#pragma once
#include <vector>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class PlayerDI_PH;

	class EGameSDK_API PlayerHealthModule {
	public:
		union {
			buffer<0x8, PlayerDI_PH*> pPlayerDI_PH;
			buffer<0x2C, float> health;
			buffer<0x3C, float> maxHealth;
		};
		
		~PlayerHealthModule();
		static PlayerHealthModule* Get();
		
		static void EmplaceBack(PlayerHealthModule* ptr);
		static void UpdateClassAddr();
	private:
		static std::vector<PlayerHealthModule*>* playerHealthModulePtrList;

		static void SetInstance(void* instance);
	};
}