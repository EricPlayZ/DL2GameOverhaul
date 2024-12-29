#pragma once
#include <EGT\Core\Core.h>
#include <EGT\Menu\Menu.h>

namespace EGT::Menu {
	namespace Player {
		extern float playerHealth;
		extern float playerMaxHealth;
		extern float playerImmunity;
		extern float playerMaxImmunity;
		extern int oldWorldMoney;
		extern ImGui::KeyBindOption godMode;
		extern ImGui::KeyBindOption freezePlayer;
		extern ImGui::KeyBindOption unlimitedImmunity;
		extern ImGui::KeyBindOption unlimitedStamina;
		extern ImGui::KeyBindOption unlimitedItems;
		extern ImGui::KeyBindOption oneHitKill;
		extern ImGui::KeyBindOption invisibleToEnemies;
		extern ImGui::KeyBindOption disableOutOfBoundsTimer;
		extern ImGui::KeyBindOption nightrunnerMode;
		extern ImGui::KeyBindOption oneHandedMode;
		extern ImGui::KeyBindOption allowGrappleHookInSafezone;
		extern ImGui::KeyBindOption disableAirControl;
		extern ImGui::Option playerVariables;

		extern std::string saveSCRPath;
		extern std::string loadSCRFilePath;

		class Tab : MenuTab {
		public:
			Tab() : MenuTab("Player", 0) {}
			void Update() override;
			void Render() override;

			static Tab instance;
		};
	}
}