#pragma once
#include <EGT\Menu\Menu.h>
	
namespace EGT::Menu {
	namespace Debug {
		extern ImGui::Option disableLowLevelMouseHook;
		extern ImGui::Option disableVftableScanning;
		extern ImGui::Option enableDebuggingConsole;

		class Tab : MenuTab {
		public:
			Tab() : MenuTab("Debug", 6) {}
			void Init() override;
			void Update() override;
			void Render() override;

			static Tab instance;
		};
	}
}