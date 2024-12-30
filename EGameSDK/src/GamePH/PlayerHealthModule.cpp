#include <vector>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\GamePH\PlayerHealthModule.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static PlayerHealthModule* pPlayerHealthModule = nullptr;
	std::vector<PlayerHealthModule*>* PlayerHealthModule::playerHealthModulePtrList = nullptr;

	PlayerHealthModule::~PlayerHealthModule() {
		delete playerHealthModulePtrList;
		playerHealthModulePtrList = nullptr;
	}

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

	void PlayerHealthModule::EmplaceBack(PlayerHealthModule* ptr) {
		if (!playerHealthModulePtrList)
			playerHealthModulePtrList = new std::vector<PlayerHealthModule*>();

		playerHealthModulePtrList->emplace_back(ptr);
	}
	void PlayerHealthModule::UpdateClassAddr() {
		if (!playerHealthModulePtrList)
			playerHealthModulePtrList = new std::vector<PlayerHealthModule*>();

		PlayerDI_PH* pPlayerDI_PH = PlayerDI_PH::Get();
		if (!pPlayerDI_PH)
			return;
		if (Get() && Get()->pPlayerDI_PH == pPlayerDI_PH)
			return;

		for (auto& pPlayerHealthModule : *playerHealthModulePtrList) {
			if (pPlayerHealthModule->pPlayerDI_PH == pPlayerDI_PH) {
				SetInstance(pPlayerHealthModule);
				playerHealthModulePtrList->clear();
				playerHealthModulePtrList->emplace_back(pPlayerHealthModule);
			}
		}
	}

	void PlayerHealthModule::SetInstance(void* instance) {
		pPlayerHealthModule = reinterpret_cast<PlayerHealthModule*>(instance);
	}
}