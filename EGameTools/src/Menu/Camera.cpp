#include <ImGui\imgui_hotkey.h>
#include <ImGui\imguiex.h>
#include <EGSDK\Engine\CVideoSettings.h>
#include <EGSDK\GamePH\FreeCamera.h>
#include <EGSDK\GamePH\GameDI_PH.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\GamePH\PlayerVariables.h>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\GamePH\GamePH_Misc.h>
#include <EGSDK\Offsets.h>
#include <EGT\Engine\Engine_Hooks.h>
#include <EGT\Menu\Camera.h>
#include <EGT\Menu\Menu.h>

namespace EGT::Menu {
	namespace Camera {
		EGSDK::Vector3 cameraOffset{};
		int FOV = 57;

		ImGui::Option photoMode{ false };

		ImGui::KeyBindOption freeCam{ VK_F3 };
		float freeCamSpeed = 2.0f;
		ImGui::KeyBindOption teleportPlayerToCamera{ VK_F4 };

		ImGui::KeyBindOption thirdPersonCamera{ VK_F1 };
		ImGui::KeyBindOption tpUseTPPModel{ VK_F2 };
		float tpDistanceBehindPlayer = 2.0f;
		float tpHeightAbovePlayer = 1.35f;
		float tpHorizontalDistanceFromPlayer = 0.0f;

		float lensDistortion = 20.0f;
		static float altLensDistortion = 20.0f;
		ImGui::KeyBindOption goProMode{ VK_NONE };
		ImGui::KeyBindOption disableSafezoneFOVReduction{ VK_NONE };
		ImGui::KeyBindOption disablePhotoModeLimits{ VK_NONE };
		ImGui::KeyBindOption disableHeadCorrection{ VK_NONE };

		static constexpr int baseFOV = 57;
		static constexpr float baseSafezoneFOVReduction = -10.0f;
		static constexpr float baseSprintHeadCorrectionFactor = 0.55f;

		static void UpdateFOV() {
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			EGSDK::Engine::CBaseCamera* viewCam = nullptr;
			if (iLevel)
				viewCam = reinterpret_cast<EGSDK::Engine::CBaseCamera*>(iLevel->GetViewCamera());

			static int previousFOV = FOV;
			
			if (goProMode.GetValue()) {
				if (goProMode.HasChangedTo(true)) {
					previousFOV = FOV;
					goProMode.SetPrevValue(true);
				}

				if (iLevel && viewCam)
					viewCam->SetFOV(110.0f);
				FOV = 110;
				return;
			} else if (goProMode.HasChangedTo(false)) {
				FOV = previousFOV;
				goProMode.SetPrevValue(false);

				if (iLevel && viewCam)
					viewCam->SetFOV(static_cast<float>(FOV));
			}

			EGSDK::Engine::CVideoSettings* videoSettings = EGSDK::Engine::CVideoSettings::Get();
			if (!videoSettings || goProMode.GetValue() || menuToggle.GetValue())
				return;

			FOV = static_cast<int>(videoSettings->extraFOV) + baseFOV;
		}
		static void FreeCamUpdate() {
			if (photoMode.GetValue())
				return;
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;
			void* viewCam = iLevel->GetViewCamera();
			if (!viewCam)
				return;
			EGSDK::GamePH::GameDI_PH* pGameDI_PH = EGSDK::GamePH::GameDI_PH::Get();
			if (!pGameDI_PH)
				return;
			EGSDK::GamePH::FreeCamera* pFreeCam = EGSDK::GamePH::FreeCamera::Get();
			if (!pFreeCam)
				return;

			static bool prevFreeCam = freeCam.GetValue();
			static bool prevEanbleSpeedMultiplier = pFreeCam->enableSpeedMultiplier1;
			static float prevSpeedMultiplier = pFreeCam->speedMultiplier;
			static float prevFOV = pFreeCam->FOV;
			if (freeCam.GetValue() && !iLevel->IsTimerFrozen()) {
				if (viewCam == pFreeCam) {
					pFreeCam->enableSpeedMultiplier1 = true;

					if (ImGui::KeyBindOption::scrolledMouseWheelUp) {
						ImGui::KeyBindOption::scrolledMouseWheelUp = false;
						freeCamSpeed += 0.1f;
					} else if (ImGui::KeyBindOption::scrolledMouseWheelDown) {
						ImGui::KeyBindOption::scrolledMouseWheelDown = false;
						freeCamSpeed -= 0.1f;
					}

					if (freeCamSpeed < 0.1f)
						freeCamSpeed = 0.1f;
					else if (freeCamSpeed > 200.0f)
						freeCamSpeed = 200.0f;

					pFreeCam->speedMultiplier = freeCamSpeed;
					pFreeCam->FOV = static_cast<float>(FOV);

					if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
						pFreeCam->speedMultiplier *= 2.0f;
					else if (ImGui::IsKeyDown(ImGuiKey_LeftAlt))
						pFreeCam->speedMultiplier /= 2.0f;

					pFreeCam->GetPosition(&EGT::Engine::Hooks::freeCamPosBeforeGamePause);

					return;
				}

				prevEanbleSpeedMultiplier = pFreeCam->enableSpeedMultiplier1;
				prevSpeedMultiplier = pFreeCam->speedMultiplier;
				prevFOV = pFreeCam->FOV;

				pGameDI_PH->TogglePhotoMode();
				pFreeCam->AllowCameraMovement(2);
			} else {
				Engine::Hooks::switchedFreeCamByGamePause = freeCam.GetValue() && iLevel->IsTimerFrozen();

				if (prevFreeCam) {
					pFreeCam->enableSpeedMultiplier1 = prevEanbleSpeedMultiplier;
					pFreeCam->speedMultiplier = prevSpeedMultiplier;
					pFreeCam->FOV = prevFOV;
				}
				if (viewCam != pFreeCam)
					return;

				pGameDI_PH->TogglePhotoMode();
				pFreeCam->AllowCameraMovement(0);
			}

			prevFreeCam = freeCam.GetValue();
		}
		static void UpdateTPPModel() {
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;

			EGSDK::GamePH::PlayerDI_PH* pPlayerDI_PH = EGSDK::GamePH::PlayerDI_PH::Get();
			if (pPlayerDI_PH) {
				if (Menu::Camera::freeCam.GetValue() && !iLevel->IsTimerFrozen())
					EGSDK::GamePH::ShowTPPModel(true);
				else if (Menu::Camera::freeCam.GetValue() && iLevel->IsTimerFrozen() && !photoMode.GetValue())
					EGSDK::GamePH::ShowTPPModel(false);
				else if (Menu::Camera::thirdPersonCamera.GetValue() && Menu::Camera::tpUseTPPModel.GetValue())
					EGSDK::GamePH::ShowTPPModel(true);
				else if (!photoMode.GetValue())
					EGSDK::GamePH::ShowTPPModel(false);
			}
		}
		static void PlayerVarsUpdate() {
			if (!EGSDK::GamePH::PlayerVariables::gotPlayerVars)
				return;

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("CameraDefaultFOVReduction", 0.0f, baseSafezoneFOVReduction, disableSafezoneFOVReduction.GetValue(), true);

			static float prevLensDistortion = lensDistortion;
			static bool lensDistortionJustEnabled = false;
			if (goProMode.GetValue()) {
				if (!lensDistortionJustEnabled) {
					prevLensDistortion = lensDistortion;
					lensDistortionJustEnabled = true;
				}
				altLensDistortion = 100.0f;
			} else if (lensDistortionJustEnabled) {
				altLensDistortion = prevLensDistortion;
				lensDistortionJustEnabled = false;
			}
			EGSDK::GamePH::PlayerVariables::ChangePlayerVar("FOVCorrection", goProMode.GetValue() ? (altLensDistortion / 100.0f) : (lensDistortion / 100.0f));

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("SprintHeadCorrectionFactor", 0.0f, baseSprintHeadCorrectionFactor, goProMode.GetValue() ? goProMode.GetValue() : disableHeadCorrection.GetValue(), true);
		}
		static void UpdateDisabledOptions() {
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			freeCam.SetChangesAreDisabled(!iLevel || !iLevel->IsLoaded() || photoMode.GetValue());
			thirdPersonCamera.SetChangesAreDisabled(freeCam.GetValue() || photoMode.GetValue());
			tpUseTPPModel.SetChangesAreDisabled(freeCam.GetValue() || photoMode.GetValue());
		}
		static void HandleToggles() {
			if (goProMode.HasChanged()) {
				goProMode.SetPrevValue(goProMode.GetValue());
				EGSDK::GamePH::ReloadJumps();
			}
			if (disableHeadCorrection.HasChanged()) {
				disableHeadCorrection.SetPrevValue(disableHeadCorrection.GetValue());
				EGSDK::GamePH::ReloadJumps();
			}
		}

		Tab Tab::instance{};
		void Tab::Update() {
			UpdateFOV();
			FreeCamUpdate();
			UpdateTPPModel();
			PlayerVarsUpdate();
			UpdateDisabledOptions();
			HandleToggles();
		}
		void Tab::Render() {
			ImGui::SeparatorText("Camera");
			

			ImGui::SeparatorText("Free Camera");
			ImGui::BeginDisabled(freeCam.GetChangesAreDisabled() || photoMode.GetValue());
			ImGui::CheckboxHotkey("Enabled##FreeCam", &freeCam, "Enables free camera which allows you to travel anywhere with the camera");
			ImGui::EndDisabled();

			ImGui::SameLine();

			ImGui::BeginDisabled(teleportPlayerToCamera.GetChangesAreDisabled());
			ImGui::CheckboxHotkey("Teleport Player to Camera", &teleportPlayerToCamera, "Teleports the player to the camera while Free Camera is activated");
			ImGui::EndDisabled();

			ImGui::SliderFloat("Speed##FreeCam", &freeCamSpeed, 0.1f, 200.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);

			ImGui::SeparatorText("Third Person Camera");
			ImGui::BeginDisabled(thirdPersonCamera.GetChangesAreDisabled());
			ImGui::CheckboxHotkey("Enabled##ThirdPerson", &thirdPersonCamera, "Enables the third person camera");
			ImGui::EndDisabled();

			ImGui::SameLine();

			ImGui::BeginDisabled(tpUseTPPModel.GetChangesAreDisabled());
			ImGui::CheckboxHotkey("Use Third Person Player (TPP) Model", &tpUseTPPModel, "Uses Aiden's TPP (Third Person Player) model while the third person camera is enabled");
			ImGui::EndDisabled();

			ImGui::SliderFloat("Distance behind player", &tpDistanceBehindPlayer, 1.0f, 10.0f, "%.2fm");
			ImGui::SliderFloat("Height above player", &tpHeightAbovePlayer, 1.0f, 3.0f, "%.2fm");
			ImGui::SliderFloat("Horizontal distance from player", &tpHorizontalDistanceFromPlayer, -2.0f, 2.0f, "%.2fm");

			ImGui::SeparatorText("Misc");
			ImGui::BeginDisabled(freeCam.GetChangesAreDisabled() || photoMode.GetValue());
			ImGui::SliderFloat("X offset", &cameraOffset.X, -10.0f, 10.0f, "%.2fm");
			ImGui::SliderFloat("Y offset", &cameraOffset.Y, -10.0f, 10.0f, "%.2fm");
			ImGui::SliderFloat("Z offset", &cameraOffset.Z, -10.0f, 10.0f, "%.2fm");
			ImGui::EndDisabled();

			auto pCVideoSettings = EGSDK::Engine::CVideoSettings::Get();
			ImGui::BeginDisabled(!pCVideoSettings || goProMode.GetValue());
			if (ImGui::SliderInt("FOV", "Camera Field of View", &FOV, 20, 160) && pCVideoSettings)
				pCVideoSettings->extraFOV = static_cast<float>(FOV - baseFOV);
			else if (pCVideoSettings && !goProMode.GetValue())
				FOV = static_cast<int>(pCVideoSettings->extraFOV) + baseFOV;
			ImGui::EndDisabled();

			ImGui::BeginDisabled(goProMode.GetValue());
			ImGui::SliderFloat("Lens Distortion", "Default game value is 20%", goProMode.GetValue() ? &altLensDistortion : &lensDistortion, 0.0f, 100.0f, "%.1f%%");
			ImGui::EndDisabled();

			ImGui::CheckboxHotkey("GoPro Mode *", &goProMode, "Makes the camera behave similar to a GoPro mounted on the forehead");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("Disable Safezone FOV Reduction", &disableSafezoneFOVReduction, "Disables the FOV reduction that happens while you're in a safezone");
			ImGui::CheckboxHotkey("Disable Photo Mode Limits", &disablePhotoModeLimits, "Disables the invisible box while in Photo Mode");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("Disable Head Correction", &disableHeadCorrection, "Disables centering of the player's hands to the center of the camera");

			ImGui::Separator();
			ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(IM_COL32(200, 0, 0, 255)), "* GoPro Mode is best used with Head Bob Reduction set to 0 and Player FOV\nCorrection set to 0 in game options");
		}
	}
}