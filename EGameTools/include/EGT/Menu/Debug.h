#pragma once
#include <EGT\Menu\Menu.h>
	
namespace EGT::Menu {
	namespace Debug {
		extern bool disableLowLevelMouseHook;

		class Tab : MenuTab {
		public:
			Tab() : MenuTab("Debug", 6) {}
			void Update() override;
			void Render() override;

			static Tab instance;
		};
	}
}