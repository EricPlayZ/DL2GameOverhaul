#pragma once
#include <EGSDK\GamePH\TimeWeather\EWeather.h>
#include <EGT\Menu\Menu.h>
	
namespace EGT::Menu {
	namespace World {
		extern float time;
		extern float gameSpeed;
		extern ImGui::KeyBindOption freezeTime;
		extern ImGui::KeyBindOption slowMotion;
		extern float slowMotionSpeed;
		extern float slowMotionTransitionTime;

		extern EGSDK::GamePH::TimeWeather::EWeather weather;

		class Tab : MenuTab {
		public:
			Tab() : MenuTab("World", 5) {}
			void Init() override;
			void Update() override;
			void Render() override;

			static Tab instance;
		};
	}
}