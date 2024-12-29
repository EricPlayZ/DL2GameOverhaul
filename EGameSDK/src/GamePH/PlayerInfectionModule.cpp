#include <vector>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\GamePH\PlayerInfectionModule.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static PlayerInfectionModule* pPlayerInfectionModule = nullptr;
	std::vector<PlayerInfectionModule*> PlayerInfectionModule::playerInfectionModulePtrList{};

	static PlayerInfectionModule* GetOffset_PlayerInfectionModule() {
		if (!pPlayerInfectionModule)
			return nullptr;
		if (!*reinterpret_cast<void**>(pPlayerInfectionModule))
			return nullptr;
		return pPlayerInfectionModule;
	}
	PlayerInfectionModule* PlayerInfectionModule::Get() {
		return _SafeGetter<PlayerInfectionModule>(GetOffset_PlayerInfectionModule, nullptr, false, nullptr);
	}
	void PlayerInfectionModule::Set(void* instance) { pPlayerInfectionModule = reinterpret_cast<PlayerInfectionModule*>(instance); }

	void PlayerInfectionModule::UpdateClassAddr() {
		PlayerDI_PH* pPlayerDI_PH = PlayerDI_PH::Get();
		if (!pPlayerDI_PH)
			return;
		if (PlayerInfectionModule::Get() && PlayerInfectionModule::Get()->pPlayerDI_PH == pPlayerDI_PH)
			return;

		for (auto& pPlayerInfectionModule : PlayerInfectionModule::playerInfectionModulePtrList) {
			if (pPlayerInfectionModule->pPlayerDI_PH == pPlayerDI_PH) {
				PlayerInfectionModule::Set(pPlayerInfectionModule);
				PlayerInfectionModule::playerInfectionModulePtrList.clear();
				PlayerInfectionModule::playerInfectionModulePtrList.emplace_back(pPlayerInfectionModule);
			}
		}
	}
}