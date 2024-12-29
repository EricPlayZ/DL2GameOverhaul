#pragma once
#include <EGT\Core\Core.h>
#include <EGT\Menu\Menu.h>

namespace EGT::Menu {
	namespace Camera {
		extern int FOV;
		
		extern ImGui::Option photoMode;

		extern ImGui::KeyBindOption freeCam;
		extern float freeCamSpeed;
		extern ImGui::KeyBindOption teleportPlayerToCamera;

		extern ImGui::KeyBindOption thirdPersonCamera;
		extern ImGui::KeyBindOption tpUseTPPModel;
		extern float tpDistanceBehindPlayer;
		extern float tpHeightAbovePlayer;
		extern float tpHorizontalDistanceFromPlayer;

		extern float lensDistortion;
		extern ImGui::KeyBindOption goProMode;
		extern ImGui::KeyBindOption disableSafezoneFOVReduction;
		extern ImGui::KeyBindOption disablePhotoModeLimits;
		extern ImGui::KeyBindOption disableHeadCorrection;

		class Tab : MenuTab {
		public:
			Tab() : MenuTab("Camera", 2) {}
			void Update() override;
			void Render() override;

			static Tab instance;
		};
	}
}