#include <pch.h>
#include "PlayerDI_PH.h"
#include "PlayerInfectionModule.h"

namespace GamePH {
	static PlayerInfectionModule* pPlayerInfectionModule = nullptr;
	std::vector<PlayerInfectionModule*> PlayerInfectionModule::playerInfectionModulePtrList{};

	static PlayerInfectionModule* GetOffset_PlayerInfectionModule() {
		if (!pPlayerInfectionModule)
			return nullptr;
		if (!*reinterpret_cast<LPVOID*>(pPlayerInfectionModule))
			return nullptr;
		return pPlayerInfectionModule;
	}
	SafeGetterDepCustom(PlayerInfectionModule, GetOffset_PlayerInfectionModule, false, nullptr)
	void PlayerInfectionModule::Set(LPVOID instance) { pPlayerInfectionModule = reinterpret_cast<PlayerInfectionModule*>(instance); }

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