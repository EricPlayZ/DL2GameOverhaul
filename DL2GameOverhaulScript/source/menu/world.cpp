#include <Hotkey.h>
#include <ImGuiEx.h>
#include <imgui.h>
#include "..\game_classes.h"
#include "world.h"

namespace Menu {
	namespace World {
		float time = 0.0f;
		static float timeBeforeFreeze = 0.0f;
		float gameSpeed = 1.0f;
		static float gameSpeedBeforeSlowMo = gameSpeed;
		KeyBindOption freezeTime{ VK_NONE };
		KeyBindOption slowMotion{ 'X' };
		float slowMotionSpeed = 0.4f;
		static float slowMotionSpeedLerp = gameSpeed;
		float slowMotionTransitionTime = 1.0f;

		EWeather::TYPE weather = EWeather::TYPE::Default;
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
			GamePH::DayNightCycle* dayNightCycle = GamePH::DayNightCycle::Get();
			GamePH::LevelDI* iLevel = GamePH::LevelDI::Get();
			freezeTime.SetChangesAreDisabled(!iLevel || !iLevel->IsLoaded() || !dayNightCycle);
			slowMotion.SetChangesAreDisabled(!iLevel || !iLevel->IsLoaded() || !dayNightCycle);
		}

		Tab Tab::instance{};
		void Tab::Update() {
			GamePH::DayNightCycle* dayNightCycle = GamePH::DayNightCycle::Get();
			if (!dayNightCycle)
				return;
			GamePH::LevelDI* iLevel = GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;

			if (freezeTime.HasChangedTo(true)) {
				timeBeforeFreeze = time;
				freezeTime.SetPrevValue(true);
			} else if (freezeTime.HasChangedTo(false)) {
				dayNightCycle->SetDaytime(timeBeforeFreeze);
				freezeTime.SetPrevValue(false);
			}

			if (slowMotion.HasChangedTo(false)) {
				static bool slowMoHasChanged = true;
				slowMotionSpeedLerp = ImGui::AnimateLerp("slowMotionSpeedLerp", slowMotionSpeed, gameSpeedBeforeSlowMo, slowMotionTransitionTime, slowMoHasChanged, &ImGui::AnimEaseOutSine);
				iLevel->TimerSetSpeedUp(slowMotionSpeedLerp);
				slowMoHasChanged = false;

				if (Utils::are_same(gameSpeed, gameSpeedBeforeSlowMo)) {
					slowMoHasChanged = true;
					slowMotion.SetPrevValue(false);
				}
			} else if (slowMotion.GetValue()) {
				if (slowMotion.HasChanged()) {
					gameSpeedBeforeSlowMo = gameSpeed;
					slowMotionSpeedLerp = gameSpeed;
				}
				slowMotionSpeedLerp = ImGui::AnimateLerp("slowMotionSpeedLerp", gameSpeedBeforeSlowMo, slowMotionSpeed, slowMotionTransitionTime, slowMotion.HasChanged(), &ImGui::AnimEaseOutSine);
				iLevel->TimerSetSpeedUp(slowMotionSpeedLerp);
				if (slowMotion.HasChanged())
					slowMotion.SetPrevValue(slowMotion.GetValue());
			}

			if (!menuToggle.GetValue()) {
				time = dayNightCycle->time1 * 24;
				if (freezeTime.GetValue() && !Utils::are_same(time, timeBeforeFreeze, 0.005f))
					dayNightCycle->SetDaytime(timeBeforeFreeze);

				if (!slowMotion.GetValue() && !slowMotion.HasChanged())
					iLevel->TimerSetSpeedUp(gameSpeed);
				if (!Utils::are_same(iLevel->TimerGetSpeedUp(), 1.0f))
					gameSpeed = iLevel->TimerGetSpeedUp();
			}
		}
		void Tab::Render() {
			GamePH::DayNightCycle* dayNightCycle = GamePH::DayNightCycle::Get();
			GamePH::LevelDI* iLevel = GamePH::LevelDI::Get();
			ImGui::SeparatorText("Time##World");
			ImGui::BeginDisabled(!iLevel || !iLevel->IsLoaded() || !dayNightCycle); {
				static bool timeSliderBeingPressed = false;
				if (ImGui::SliderFloat("Time", &time, 0.01f, 24.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
					timeSliderBeingPressed = true;
				else if (iLevel && iLevel->IsLoaded() && dayNightCycle) {
					if (timeSliderBeingPressed) {
						timeSliderBeingPressed = false;
						timeBeforeFreeze = time;
						dayNightCycle->SetDaytime(time);
					}
					time = dayNightCycle->time1 * 24;
					if (freezeTime.GetValue() && !Utils::are_same(time, timeBeforeFreeze, 0.005f))
						dayNightCycle->SetDaytime(timeBeforeFreeze);
				}

				ImGui::BeginDisabled(slowMotion.GetValue()); {
					if (ImGui::SliderFloat("Game Speed", &gameSpeed, 0.0f, 2.0f, "%.2fx"))
						iLevel->TimerSetSpeedUp(gameSpeed);
					else if (iLevel && iLevel->IsLoaded()) {
						if (!slowMotion.GetValue() && !slowMotion.HasChanged())
							iLevel->TimerSetSpeedUp(gameSpeed);
						if (!Utils::are_same(iLevel->TimerGetSpeedUp(), 1.0f))
							gameSpeed = iLevel->TimerGetSpeedUp();
					}
					ImGui::EndDisabled();
				}

				ImGui::CheckboxHotkey("Freeze Time", &freezeTime);
				ImGui::SameLine();
				ImGui::CheckboxHotkey("Slow Motion", &slowMotion);

				ImGui::EndDisabled();
			}
			ImGui::SliderFloat("Slow Motion Speed", &slowMotionSpeed, 0.01f, 0.99f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SliderFloat("Slow Motion Transition Time", &slowMotionTransitionTime, 0.00f, 5.00f, "%.2fs", ImGuiSliderFlags_AlwaysClamp);

			GamePH::TimeWeather::CSystem* timeWeatherSystem = GamePH::TimeWeather::CSystem::Get();
			const bool weatherDisabledFlag = !iLevel || !iLevel->IsLoaded() || !timeWeatherSystem;

			ImGui::SeparatorText("Weather##World");
			ImGui::BeginDisabled(weatherDisabledFlag); {
				if (ImGui::Combo("Weather", reinterpret_cast<int*>(&weather), weatherItems, IM_ARRAYSIZE(weatherItems)) && timeWeatherSystem)
					timeWeatherSystem->SetForcedWeather(static_cast<EWeather::TYPE>(weather - 1));
				ImGui::Text("Setting weather to: %s", !weatherDisabledFlag ? weatherItems[weather] : "");
				ImGui::Text("Current weather: %s", !weatherDisabledFlag ? weatherItems[timeWeatherSystem->GetCurrentWeather() + 1] : "");
				ImGui::EndDisabled();
			}
		}
	}
}