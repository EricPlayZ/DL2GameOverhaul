#include <ImGui\imgui_hotkey.h>
#include <ImGui\imguiex.h>
#include <ImGui\imguiex_animation.h>
#include <EGSDK\GamePH\DayNightCycle.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\GamePH\PlayerVariables.h>
#include <EGSDK\GamePH\TimeWeather\CSystem.h>
#include <EGSDK\GamePH\TimeWeather\EWeather.h>
#include <EGT\Menu\World.h>

namespace EGT::Menu {
	namespace World {
		float time = 0.0f;
		static float timeBeforeFreeze = 0.0f;
		float gameSpeed = 1.0f;
		static bool isModifyingGameSpeed = false;
		static float actualGameSpeed = gameSpeed;
		static float gameSpeedBeforeSlowMo = gameSpeed;
		ImGui::KeyBindOption freezeTime{ VK_NONE };
		ImGui::KeyBindOption slowMotion{ '4' };
		float slowMotionSpeed = 0.4f;
		static float slowMotionSpeedLerp = gameSpeed;
		float slowMotionTransitionTime = 1.0f;

		EGSDK::GamePH::TimeWeather::EWeather::TYPE weather = EGSDK::GamePH::TimeWeather::EWeather::TYPE::Default;
		static const char* const weatherItems[7] = {
			"Default",
			"Foggy",
			"Clear",
			"Overcast",
			"Cloudy",
			"Rainy",
			"Stormy"
		};

		static void UpdateDisabledOptions() {
			EGSDK::GamePH::DayNightCycle* dayNightCycle = EGSDK::GamePH::DayNightCycle::Get();
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			freezeTime.SetChangesAreDisabled(!iLevel || !iLevel->IsLoaded() || !dayNightCycle);
			slowMotion.SetChangesAreDisabled(!iLevel || !iLevel->IsLoaded() || !dayNightCycle);
		}

		Tab Tab::instance{};
		void Tab::Update() {
			EGSDK::GamePH::DayNightCycle* dayNightCycle = EGSDK::GamePH::DayNightCycle::Get();
			if (!dayNightCycle)
				return;
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;

			if (freezeTime.HasChangedTo(true)) {
				timeBeforeFreeze = time;
				freezeTime.SetPrevValue(true);
			} else if (freezeTime.HasChangedTo(false)) {
				dayNightCycle->SetDaytime(timeBeforeFreeze);
				freezeTime.SetPrevValue(false);
			}

			static bool slowMoHasChanged = true;
			if (slowMotion.HasChangedTo(false)) {
				static float gameSpeedAfterChange = 0.0f;
				if (slowMoHasChanged)
					gameSpeedAfterChange = actualGameSpeed;

				slowMotionSpeedLerp = ImGui::AnimateLerp("slowMotionSpeedLerp", gameSpeedAfterChange, gameSpeedBeforeSlowMo, slowMotionTransitionTime, slowMoHasChanged, &ImGui::AnimEaseInOutSine);
				iLevel->TimerSetSpeedUp(slowMotionSpeedLerp);
				slowMoHasChanged = false;

				if (EGSDK::Utils::Values::are_samef(actualGameSpeed, gameSpeedBeforeSlowMo)) {
					slowMoHasChanged = true;
					slowMotion.SetPrevValue(false);
				}
			} else if (slowMotion.GetValue()) {
				static float gameSpeedAfterChange = 0.0f;
				if (slowMotion.HasChanged()) {
					if (slowMoHasChanged)
						gameSpeedBeforeSlowMo = actualGameSpeed;
					gameSpeedAfterChange = actualGameSpeed;
				}

				slowMotionSpeedLerp = ImGui::AnimateLerp("slowMotionSpeedLerp", gameSpeedAfterChange, slowMotionSpeed, slowMotionTransitionTime, slowMotion.HasChanged(), &ImGui::AnimEaseInOutSine);
				iLevel->TimerSetSpeedUp(slowMotionSpeedLerp);

				if (slowMotion.HasChanged()) {
					slowMoHasChanged = true;
					slowMotion.SetPrevValue(slowMotion.GetValue());
				}
			}

			time = dayNightCycle->time1 * 24.0f;
			if (freezeTime.GetValue() && !EGSDK::Utils::Values::are_samef(time, timeBeforeFreeze, 0.0095f))
				dayNightCycle->SetDaytime(timeBeforeFreeze);

			if (!isModifyingGameSpeed) {
				if (!slowMotion.GetValue() && !slowMotion.HasChanged() && !EGSDK::Utils::Values::are_samef(gameSpeed, 1.0f))
					iLevel->TimerSetSpeedUp(gameSpeed);
				actualGameSpeed = iLevel->TimerGetSpeedUp();
			}
		}
		void Tab::Render() {
			EGSDK::GamePH::DayNightCycle* dayNightCycle = EGSDK::GamePH::DayNightCycle::Get();
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			ImGui::SeparatorText("Time##World");
			ImGui::BeginDisabled(!iLevel || !iLevel->IsLoaded() || !dayNightCycle); {
				static bool previousAntizinDrainBlocked = false;
				static bool haveResetAntizinDrainBlocked = true;
				if (ImGui::SliderFloat("Time", &time, 0.01f, 24.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
					if (haveResetAntizinDrainBlocked)
						previousAntizinDrainBlocked = EGSDK::GamePH::PlayerVariables::GetPlayerVarValue<bool>("AntizinDrainBlocked");
					EGSDK::GamePH::PlayerVariables::ChangePlayerVar("AntizinDrainBlocked", true);
					haveResetAntizinDrainBlocked = false;

					timeBeforeFreeze = time;
					dayNightCycle->SetDaytime(time);
				} else if (iLevel && iLevel->IsLoaded() && dayNightCycle) {
					if (!haveResetAntizinDrainBlocked) {
						EGSDK::GamePH::PlayerVariables::ChangePlayerVar("AntizinDrainBlocked", previousAntizinDrainBlocked);
						haveResetAntizinDrainBlocked = true;
					}
				}

				ImGui::BeginDisabled(slowMotion.GetValue()); {
					isModifyingGameSpeed = ImGui::SliderFloat("Game Speed", &gameSpeed, 0.0f, 2.0f, "%.2fx");
					if (isModifyingGameSpeed)
						iLevel->TimerSetSpeedUp(gameSpeed);
					ImGui::EndDisabled();
				}

				ImGui::CheckboxHotkey("Freeze Time", &freezeTime, "Freezes time");
				ImGui::SameLine();
				ImGui::CheckboxHotkey("Slow Motion", &slowMotion, "Slows the game down to the speed specified on the \"Slow Motion Speed\" slider");

				ImGui::EndDisabled();
			}
			ImGui::SliderFloat("Slow Motion Speed", &slowMotionSpeed, 0.01f, 0.99f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SliderFloat("Slow Motion Transition Time", &slowMotionTransitionTime, 0.00f, 5.00f, "%.2fs", ImGuiSliderFlags_AlwaysClamp);

			EGSDK::GamePH::TimeWeather::CSystem* timeWeatherSystem = EGSDK::GamePH::TimeWeather::CSystem::Get();
			const bool weatherDisabledFlag = !iLevel || !iLevel->IsLoaded() || !timeWeatherSystem;

			ImGui::SeparatorText("Weather##World");
			ImGui::BeginDisabled(weatherDisabledFlag); {
				if (ImGui::Combo("Weather", reinterpret_cast<int*>(&weather), weatherItems, IM_ARRAYSIZE(weatherItems)) && timeWeatherSystem)
					timeWeatherSystem->SetForcedWeather(static_cast<EGSDK::GamePH::TimeWeather::EWeather::TYPE>(weather - 1));
				ImGui::Text("Setting weather to: %s", !weatherDisabledFlag ? weatherItems[weather] : "");
				ImGui::Text("Current weather: %s", !weatherDisabledFlag ? weatherItems[timeWeatherSystem->GetCurrentWeather() + 1] : "");
				ImGui::EndDisabled();
			}
		}
	}
}