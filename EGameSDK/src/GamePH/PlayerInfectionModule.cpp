#include <vector>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\GamePH\PlayerInfectionModule.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static PlayerInfectionModule* pPlayerInfectionModule = nullptr;
	std::vector<PlayerInfectionModule*>* PlayerInfectionModule::playerInfectionModulePtrList = nullptr;

	PlayerInfectionModule::~PlayerInfectionModule() {
		delete playerInfectionModulePtrList;
		playerInfectionModulePtrList = nullptr;
	}

	static PlayerInfectionModule* GetOffset_PlayerInfectionModule() {
		if (!pPlayerInfectionModule)
			return nullptr;
		if (!*reinterpret_cast<void**>(pPlayerInfectionModule))
			return nullptr;
		return pPlayerInfectionModule;
	}
	PlayerInfectionModule* PlayerInfectionModule::Get() {
		return ClassHelpers::SafeGetter<PlayerInfectionModule>(GetOffset_PlayerInfectionModule, false, false);
	}

	void PlayerInfectionModule::EmplaceBack(PlayerInfectionModule* ptr) {
		if (!playerInfectionModulePtrList)
			playerInfectionModulePtrList = new std::vector<PlayerInfectionModule*>();

		playerInfectionModulePtrList->emplace_back(ptr);
	}
	void PlayerInfectionModule::UpdateClassAddr() {
		if (!playerInfectionModulePtrList)
			playerInfectionModulePtrList = new std::vector<PlayerInfectionModule*>();

		PlayerDI_PH* pPlayerDI_PH = PlayerDI_PH::Get();
		if (!pPlayerDI_PH)
			return;
		if (PlayerInfectionModule::Get() && PlayerInfectionModule::Get()->pPlayerDI_PH == pPlayerDI_PH)
			return;

		for (auto& pPlayerInfectionModule : *playerInfectionModulePtrList) {
			if (pPlayerInfectionModule->pPlayerDI_PH == pPlayerDI_PH) {
				SetInstance(pPlayerInfectionModule);
				playerInfectionModulePtrList->clear();
				playerInfectionModulePtrList->emplace_back(pPlayerInfectionModule);
			}
		}
	}

	void PlayerInfectionModule::SetInstance(void* instance) {
		pPlayerInfectionModule = reinterpret_cast<PlayerInfectionModule*>(instance);
	}
}