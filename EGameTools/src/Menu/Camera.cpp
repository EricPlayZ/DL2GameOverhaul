#include <ImGui\imgui_hotkey.h>
#include <ImGui\imguiex.h>
#include <ImGui\imguiex_animation.h>
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
		static constexpr float baseFOV = 57;
		static constexpr float baseSafezoneFOVReduction = -10.0f;
		static constexpr float baseSprintHeadCorrectionFactor = 0.55f;

		EGSDK::Vector3 cameraOffset{};
		float firstPersonFOV = baseFOV;
		ImGui::KeyBindOption firstPersonZoomIn{ 'Q' , false };
		static bool isZoomingIn = false;

		ImGui::Option photoMode{ false };

		ImGui::KeyBindOption freeCam{ VK_F3 };
		float freeCamFOV = baseFOV;
		float freeCamSpeed = 2.0f;
		ImGui::KeyBindOption teleportPlayerToCamera{ VK_F4 };

		ImGui::KeyBindOption thirdPersonCamera{ VK_F1 };
		ImGui::KeyBindOption tpUseTPPModel{ VK_F2 };
		float thirdPersonFOV = baseFOV;
		float thirdPersonDistanceBehindPlayer = 2.0f;
		float thirdPersonHeightAbovePlayer = 1.35f;
		float thirdPersonHorizontalDistanceFromPlayer = 0.0f;

		float lensDistortion = 20.0f;
		static float altLensDistortion = lensDistortion;
		ImGui::KeyBindOption goProMode{ VK_NONE };
		ImGui::KeyBindOption disableSafezoneFOVReduction{ VK_NONE };
		ImGui::KeyBindOption disablePhotoModeLimits{ VK_NONE };
		ImGui::KeyBindOption disableHeadCorrection{ VK_NONE };

		static void UpdateFirstPersonFOV() {
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			auto viewCam = iLevel ? reinterpret_cast<EGSDK::Engine::CBaseCamera*>(iLevel->GetViewCamera()) : nullptr;
			{
				static float previousFirstPersonFOV = firstPersonFOV;

				if (goProMode.GetValue()) {
					if (goProMode.HasChangedTo(true)) {
						previousFirstPersonFOV = firstPersonFOV;
						goProMode.SetPrevValue(true);
					}

					if (iLevel && viewCam)
						viewCam->SetFOV(110.0f);
					firstPersonFOV = 110;
					return;
				} else if (goProMode.HasChangedTo(false)) {
					firstPersonFOV = previousFirstPersonFOV;
					goProMode.SetPrevValue(false);

					if (iLevel && viewCam)
						viewCam->SetFOV(firstPersonFOV);
				}
			}
			
			static float originalFirstPersonFOV = firstPersonFOV;
			static float previousFirstPersonFOV = firstPersonFOV;
			static bool scrolledDown = false;
			static bool hasChangedZoomLevel = false;
			static int zoomLevel = 0;
			if (iLevel && viewCam && !thirdPersonCamera.GetValue() && !freeCam.GetValue()) {
				if (firstPersonZoomIn.IsKeyDown()) {
					if (firstPersonZoomIn.IsKeyPressed()) {
						hasChangedZoomLevel = true;
						if (!isZoomingIn) {
							originalFirstPersonFOV = firstPersonFOV;
							previousFirstPersonFOV = originalFirstPersonFOV;
						} else
							previousFirstPersonFOV = firstPersonFOV;
					}

					isZoomingIn = true;
					float newFOV = previousFirstPersonFOV;
					switch (zoomLevel) {
						case 0:
							newFOV = std::max(originalFirstPersonFOV - 25.0f, 42.0f);
							break;
						case 1:
							newFOV = std::max(originalFirstPersonFOV - 45.0f, 25.0f);
							break;
						case 2:
							newFOV = std::max(originalFirstPersonFOV - 65.0f, 15.0f);
							break;
						default:
							break;
					}
					firstPersonFOV = ImGui::AnimateLerp("zoomInFOVLerp", previousFirstPersonFOV, newFOV, 0.3f, hasChangedZoomLevel, &ImGui::AnimEaseOutSine);
					viewCam->SetFOV(firstPersonFOV);
					hasChangedZoomLevel = false;

					if (ImGui::KeyBindOption::scrolledMouseWheelUp) {
						ImGui::KeyBindOption::scrolledMouseWheelUp = false;
						if (zoomLevel < 2) {
							scrolledDown = false;
							hasChangedZoomLevel = true;
							previousFirstPersonFOV = firstPersonFOV;
							zoomLevel++;
						}
					} else if (ImGui::KeyBindOption::scrolledMouseWheelDown) {
						ImGui::KeyBindOption::scrolledMouseWheelDown = false;
						if (zoomLevel > 0) {
							scrolledDown = true;
							hasChangedZoomLevel = true;
							previousFirstPersonFOV = firstPersonFOV;
							zoomLevel--;
						}
					}
				} else {
					zoomLevel = 0;
					scrolledDown = false;
					if (firstPersonZoomIn.IsKeyReleased()) {
						hasChangedZoomLevel = true;
						previousFirstPersonFOV = firstPersonFOV;
					}
					if (!EGSDK::Utils::Values::are_samef(firstPersonFOV, originalFirstPersonFOV) && isZoomingIn) {
						firstPersonFOV = ImGui::AnimateLerp("zoomInFOVLerp", previousFirstPersonFOV, originalFirstPersonFOV, 0.25f, hasChangedZoomLevel, &ImGui::AnimEaseOutSine);
						viewCam->SetFOV(firstPersonFOV);
						hasChangedZoomLevel = false;
					} else
						isZoomingIn = false;
				}
			}

			auto videoSettings = EGSDK::Engine::CVideoSettings::Get();
			if (videoSettings && !goProMode.GetValue() && !menuToggle.GetValue() && !isZoomingIn)
				firstPersonFOV = static_cast<int>(videoSettings->extraFOV) + baseFOV;
		}
		static void FreeCamUpdate() {
			if (photoMode.GetValue())
				return;
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;

			auto viewCam = iLevel->GetViewCamera();
			if (!viewCam)
				return;
			auto pGameDI_PH = EGSDK::GamePH::GameDI_PH::Get();
			if (!pGameDI_PH)
				return;
			auto pFreeCam = EGSDK::GamePH::FreeCamera::Get();
			if (!pFreeCam)
				return;

			static bool prevFreeCam = freeCam.GetValue();
			static bool prevEnableSpeedMultiplier = pFreeCam->enableSpeedMultiplier1;
			static float prevSpeedMultiplier = pFreeCam->speedMultiplier;
			static float prevFOV = pFreeCam->GetFOV();

			if (freeCam.GetValue() && !iLevel->IsTimerFrozen()) {
				if (viewCam == pFreeCam) {
					pFreeCam->enableSpeedMultiplier1 = true;

					if (ImGui::KeyBindOption::scrolledMouseWheelUp) {
						ImGui::KeyBindOption::scrolledMouseWheelUp = false;
						freeCamSpeed = std::min(freeCamSpeed + 0.1f, 200.0f);
					} else if (ImGui::KeyBindOption::scrolledMouseWheelDown) {
						ImGui::KeyBindOption::scrolledMouseWheelDown = false;
						freeCamSpeed = std::max(freeCamSpeed - 0.1f, 0.1f);
					}

					pFreeCam->speedMultiplier = freeCamSpeed;
					pFreeCam->SetFOV(static_cast<float>(freeCamFOV));

					if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
						pFreeCam->speedMultiplier *= 2.0f;
					else if (ImGui::IsKeyDown(ImGuiKey_LeftAlt))
						pFreeCam->speedMultiplier /= 2.0f;

					pFreeCam->GetPosition(&EGT::Engine::Hooks::freeCamPosBeforeGamePause);
					return;
				}

				prevEnableSpeedMultiplier = pFreeCam->enableSpeedMultiplier1;
				prevSpeedMultiplier = pFreeCam->speedMultiplier;
				prevFOV = pFreeCam->GetFOV();

				pGameDI_PH->TogglePhotoMode();
				pFreeCam->AllowCameraMovement(2);
			} else {
				Engine::Hooks::switchedFreeCamByGamePause = freeCam.GetValue() && iLevel->IsTimerFrozen();

				if (prevFreeCam) {
					pFreeCam->enableSpeedMultiplier1 = prevEnableSpeedMultiplier;
					pFreeCam->speedMultiplier = prevSpeedMultiplier;
					pFreeCam->SetFOV(prevFOV);
				}
				if (viewCam != pFreeCam)
					return;

				pGameDI_PH->TogglePhotoMode();
				pFreeCam->AllowCameraMovement(0);
			}

			prevFreeCam = freeCam.GetValue();
		}
		static void UpdateTPPModel() {
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;
			auto pPlayerDI_PH = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!pPlayerDI_PH)
				return;

			if (freeCam.GetValue() && !iLevel->IsTimerFrozen())
				EGSDK::GamePH::ShowTPPModel(true);
			else if (freeCam.GetValue() && iLevel->IsTimerFrozen() && !photoMode.GetValue())
				EGSDK::GamePH::ShowTPPModel(false);
			else if (thirdPersonCamera.GetValue() && tpUseTPPModel.GetValue())
				EGSDK::GamePH::ShowTPPModel(true);
			else if (!photoMode.GetValue())
				EGSDK::GamePH::ShowTPPModel(false);
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
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			freeCam.SetChangesAreDisabled(!iLevel || !iLevel->IsLoaded() || photoMode.GetValue() || isZoomingIn);
			thirdPersonCamera.SetChangesAreDisabled(freeCam.GetValue() || photoMode.GetValue() || isZoomingIn);
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
			UpdateFirstPersonFOV();
			FreeCamUpdate();
			UpdateTPPModel();
			PlayerVarsUpdate();
			UpdateDisabledOptions();
			HandleToggles();
		}
		void Tab::Render() {
			ImGui::SeparatorText("First Person Camera");
			auto pCVideoSettings = EGSDK::Engine::CVideoSettings::Get();
			ImGui::BeginDisabled(!pCVideoSettings || goProMode.GetValue() || isZoomingIn);
			if (ImGui::SliderFloat("FOV##FirstPerson", "First person camera field of view", &firstPersonFOV, 20.0f, 160.0f, "%.0f°") && pCVideoSettings)
				pCVideoSettings->extraFOV = firstPersonFOV - baseFOV;
			else if (pCVideoSettings && !goProMode.GetValue())
				firstPersonFOV = pCVideoSettings->extraFOV + baseFOV;
			ImGui::EndDisabled();
			ImGui::BeginDisabled(freeCam.GetChangesAreDisabled());
			ImGui::SetNextItemWidth(400.0f * Menu::scale);
			ImGui::SliderFloat3("Camera Offset (XYZ)", reinterpret_cast<float*>(&cameraOffset), -0.5f, 0.5f, "%.2fm");
			ImGui::EndDisabled();

			ImGui::SeparatorText("Third Person Camera");
			ImGui::BeginDisabled(thirdPersonCamera.GetChangesAreDisabled());
			ImGui::CheckboxHotkey("Enabled##ThirdPerson", &thirdPersonCamera, "Enables the third person camera");
			ImGui::EndDisabled();

			ImGui::SameLine();

			ImGui::BeginDisabled(tpUseTPPModel.GetChangesAreDisabled());
			ImGui::CheckboxHotkey("Use Third Person Player (TPP) Model", &tpUseTPPModel, "Uses Aiden's TPP (Third Person Player) model while the third person camera is enabled");
			ImGui::EndDisabled();

			ImGui::SliderFloat("FOV##ThirdPerson", "Third person camera field of view", &thirdPersonFOV, 20.0f, 160.0f, "%.0f°");
			ImGui::SliderFloat("Distance behind player", &thirdPersonDistanceBehindPlayer, 1.0f, 10.0f, "%.2fm");
			ImGui::SliderFloat("Height above player", &thirdPersonHeightAbovePlayer, 1.0f, 3.0f, "%.2fm");
			ImGui::SliderFloat("Horizontal distance from player", &thirdPersonHorizontalDistanceFromPlayer, -2.0f, 2.0f, "%.2fm");

			ImGui::SeparatorText("Free Camera");
			ImGui::BeginDisabled(freeCam.GetChangesAreDisabled() || photoMode.GetValue());
			ImGui::CheckboxHotkey("Enabled##FreeCam", &freeCam, "Enables free camera which allows you to travel anywhere with the camera");
			ImGui::EndDisabled();

			ImGui::SameLine();

			ImGui::BeginDisabled(teleportPlayerToCamera.GetChangesAreDisabled());
			ImGui::CheckboxHotkey("Teleport Player to Camera", &teleportPlayerToCamera, "Teleports the player to the camera while Free Camera is activated");
			ImGui::EndDisabled();

			ImGui::SliderFloat("FOV##FreeCam", "Free camera field of view", &freeCamFOV, 20.0f, 160.0f, "%.0f°");
			ImGui::SliderFloat("Speed##FreeCam", &freeCamSpeed, 0.1f, 200.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);

			ImGui::SeparatorText("Misc");
			ImGui::BeginDisabled(goProMode.GetValue());
			ImGui::SliderFloat("Lens Distortion", "Default game value is 20%", goProMode.GetValue() ? &altLensDistortion : &lensDistortion, 0.0f, 100.0f, "%.1f%%");
			ImGui::EndDisabled();

			ImGui::CheckboxHotkey("GoPro Mode *", &goProMode, "Makes the camera behave similar to a GoPro mounted on the forehead");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("Disable Safezone FOV Reduction", &disableSafezoneFOVReduction, "Disables the FOV reduction that happens while you're in a safezone");
			ImGui::CheckboxHotkey("Disable Photo Mode Limits", &disablePhotoModeLimits, "Disables the invisible box while in Photo Mode");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("Disable Head Correction", &disableHeadCorrection, "Disables centering of the player's hands to the center of the camera");
			ImGui::CheckboxHotkey("Zoom In", &firstPersonZoomIn);

			ImGui::Separator();
			ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(IM_COL32(200, 0, 0, 255)), "* GoPro Mode is best used with Head Bob Reduction set to 0 and Player FOV\nCorrection set to 0 in game options");
		}
	}
}