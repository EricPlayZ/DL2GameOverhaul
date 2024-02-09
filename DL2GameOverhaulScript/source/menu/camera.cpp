#include <Hotkey.h>
#include <ImGuiEx.h>
#include <imgui.h>
#include "..\core.h"
#include "..\game_classes.h"
#include "..\sigscan\offsets.h"
#include "camera.h"
#include "menu.h"

namespace Menu {
	namespace Camera {
		int FOV = 57;

		Option photoMode{};

		KeyBindOption freeCam{ VK_F3 };
		float freeCamSpeed = 2.0f;
		KeyBindOption teleportPlayerToCamera{ VK_F4 };

		KeyBindOption thirdPersonCamera{ VK_F1 };
		KeyBindOption tpUseTPPModel{ VK_F2 };
		float tpDistanceBehindPlayer = 2.0f;
		float tpHeightAbovePlayer = 1.35f;
		float tpHorizontalDistanceFromPlayer = 0.0f;

		KeyBindOption disablePhotoModeLimits{ VK_NONE };
		KeyBindOption disableSafezoneFOVReduction{ VK_NONE };

		static const int baseFOV = 57;
		static const float baseSafezoneFOVReduction = -10.0f;

		static void UpdateFOV() {
			if (menuToggle.GetValue())
				return;

			Engine::CVideoSettings* videoSettings = Engine::CVideoSettings::Get();
			if (!videoSettings)
				return;

			FOV = static_cast<int>(videoSettings->extraFOV) + baseFOV;
		}
		static void FreeCamUpdate() {
			if (photoMode.GetValue())
				return;
			GamePH::LevelDI* iLevel = GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;
			LPVOID viewCam = iLevel->GetViewCamera();
			if (!viewCam)
				return;
			GamePH::GameDI_PH* pGameDI_PH = GamePH::GameDI_PH::Get();
			if (!pGameDI_PH)
				return;
			GamePH::FreeCamera* pFreeCam = GamePH::FreeCamera::Get();
			if (!pFreeCam)
				return;

			if (freeCam.GetValue()) {
				if (viewCam == pFreeCam) {
					pFreeCam->enableSpeedMultiplier1 = true;

					if (KeyBindOption::scrolledMouseWheelUp) {
						KeyBindOption::scrolledMouseWheelUp = false;
						freeCamSpeed += 0.5f;
					} else if (KeyBindOption::scrolledMouseWheelDown) {
						KeyBindOption::scrolledMouseWheelDown = false;
						freeCamSpeed -= 0.5f;
					}

					if (freeCamSpeed < 0.1f)
						freeCamSpeed = 0.1f;
					else if (freeCamSpeed > 200.0f)
						freeCamSpeed = 200.0f;

					pFreeCam->speedMultiplier = freeCamSpeed;

					if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
						pFreeCam->speedMultiplier *= 2.0f;
					else if (ImGui::IsKeyDown(ImGuiKey_LeftAlt))
						pFreeCam->speedMultiplier /= 2.0f;

					return;
				}

				pGameDI_PH->TogglePhotoMode();
				pFreeCam->AllowCameraMovement(2);

				freeCam.SetPrevValue(true);
			} else {
				if (freeCam.GetPrevValue()) {
					pFreeCam->enableSpeedMultiplier1 = false;
					pFreeCam->speedMultiplier = 0.1f;
				}
				if (viewCam != pFreeCam)
					return;

				pGameDI_PH->TogglePhotoMode();
				pFreeCam->AllowCameraMovement(0);

				freeCam.SetPrevValue(false);
			}
		}
		static void UpdatePlayerVars() {
			if (!GamePH::PlayerVariables::gotPlayerVars)
				return;

			GamePH::PlayerVariables::ManagePlayerVarOption("CameraDefaultFOVReduction", 0.0f, baseSafezoneFOVReduction, &disableSafezoneFOVReduction);
		}
		static void UpdateDisabledOptions() {
			GamePH::LevelDI* iLevel = GamePH::LevelDI::Get();
			freeCam.SetChangesAreDisabled(!iLevel || !iLevel->IsLoaded() || photoMode.GetValue());
			teleportPlayerToCamera.SetChangesAreDisabled(!iLevel || !iLevel->IsLoaded());
			thirdPersonCamera.SetChangesAreDisabled(freeCam.GetValue() || photoMode.GetValue());
			tpUseTPPModel.SetChangesAreDisabled(freeCam.GetValue() || photoMode.GetValue());
		}

		Tab Tab::instance{};
		void Tab::Update() {
			UpdateFOV();
			FreeCamUpdate();
			UpdatePlayerVars();
			UpdateDisabledOptions();
		}
		void Tab::Render() {
			ImGui::SeparatorText("Free Camera");
			ImGui::BeginDisabled(freeCam.GetChangesAreDisabled() || photoMode.GetValue()); {
				ImGui::CheckboxHotkey("Enabled##FreeCam", &freeCam);
				ImGui::EndDisabled();
			}
			ImGui::SliderFloat("Speed##FreeCam", &freeCamSpeed, 0.1f, 200.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);
			ImGui::BeginDisabled(teleportPlayerToCamera.GetChangesAreDisabled()); {
				ImGui::CheckboxHotkey("Teleport Player to Camera", &teleportPlayerToCamera);
				ImGui::EndDisabled();
			}

			ImGui::SeparatorText("Third Person Camera");
			ImGui::BeginDisabled(thirdPersonCamera.GetChangesAreDisabled()); {
				ImGui::CheckboxHotkey("Enabled##ThirdPerson", &thirdPersonCamera);
				ImGui::EndDisabled();
			}
			ImGui::BeginDisabled(tpUseTPPModel.GetChangesAreDisabled()); {
				ImGui::CheckboxHotkey("Use Third Person Player (TPP) Model", &tpUseTPPModel);
				ImGui::EndDisabled();
			}
			ImGui::SliderFloat("Distance behind player", &tpDistanceBehindPlayer, 1.0f, 10.0f, "%.2fm");
			ImGui::SliderFloat("Height above player", &tpHeightAbovePlayer, 1.0f, 3.0f, "%.2fm");
			ImGui::SliderFloat("Horizontal distance from player", &tpHorizontalDistanceFromPlayer, -2.0f, 2.0f, "%.2fm");

			ImGui::SeparatorText("Misc");
			Engine::CVideoSettings* pCVideoSettings = Engine::CVideoSettings::Get();
			ImGui::BeginDisabled(!pCVideoSettings); {
				if (ImGui::SliderInt("FOV", &FOV, 20, 160) && pCVideoSettings)
					pCVideoSettings->extraFOV = static_cast<float>(FOV - baseFOV);
				else if (pCVideoSettings)
					FOV = static_cast<int>(pCVideoSettings->extraFOV) + baseFOV;
				ImGui::EndDisabled();
			}
			ImGui::CheckboxHotkey("Disable Photo Mode Limits", &disablePhotoModeLimits);
			ImGui::CheckboxHotkey("Disable Safezone FOV Reduction", &disableSafezoneFOVReduction);
		}
	}
}