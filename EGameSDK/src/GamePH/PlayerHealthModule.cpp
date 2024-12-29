#include <vector>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\GamePH\PlayerHealthModule.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static PlayerHealthModule* pPlayerHealthModule = nullptr;
	std::vector<PlayerHealthModule*> PlayerHealthModule::playerHealthModulePtrList{};

	static PlayerHealthModule* GetOffset_PlayerHealthModule() {
		if (!pPlayerHealthModule)
			return nullptr;
		if (!*reinterpret_cast<void**>(pPlayerHealthModule))
			return nullptr;
		return pPlayerHealthModule;
	}
	PlayerHealthModule* PlayerHealthModule::Get() {
		return _SafeGetter<PlayerHealthModule>(GetOffset_PlayerHealthModule, nullptr, false, nullptr);
	}
	void PlayerHealthModule::Set(void* instance) { pPlayerHealthModule = reinterpret_cast<PlayerHealthModule*>(instance); }

	void PlayerHealthModule::UpdateClassAddr() {
		PlayerDI_PH* pPlayerDI_PH = PlayerDI_PH::Get();
		if (!pPlayerDI_PH)
			return;
		if (PlayerHealthModule::Get() && PlayerHealthModule::Get()->pPlayerDI_PH == pPlayerDI_PH)
			return;

		for (auto& pPlayerHealthModule : PlayerHealthModule::playerHealthModulePtrList) {
			if (pPlayerHealthModule->pPlayerDI_PH == pPlayerDI_PH) {
				PlayerHealthModule::Set(pPlayerHealthModule);
				PlayerHealthModule::playerHealthModulePtrList.clear();
				PlayerHealthModule::playerHealthModulePtrList.emplace_back(pPlayerHealthModule);
			}
		}
	}
}