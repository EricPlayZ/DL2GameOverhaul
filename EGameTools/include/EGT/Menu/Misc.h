#pragma once
#include <EGT\Menu\Menu.h>
	
namespace EGT::Menu {
	namespace Misc {
		extern ImGui::KeyBindOption disableGamePauseWhileAFK;
		extern ImGui::KeyBindOption disableHUD;
		extern ImGui::KeyBindOption disableTAA;
		extern ImGui::Option disableSavegameCRCCheck;
		extern ImGui::Option disableDataPAKsCRCCheck;
		extern ImGui::Option increaseDataPAKsLimit;

		class Tab : MenuTab {
		public:
			Tab() : MenuTab("Misc", 4) {}
			void Init() override;
			void Update() override;
			void Render() override;

			static Tab instance;
		};
	}
}