#include <pch.h>
#include "..\Engine\CLevel.h"
#include "..\offsets.h"
#include "LevelDI.h"
#include "PlayerDI_PH.h"

namespace GamePH {
	Utils::Time::Timer LevelDI::loadTimer{ 20000 };
	bool LevelDI::relyOnTimer = false;
	bool LevelDI::hasLoaded = false;

	bool LevelDI::IsLoading() {
		return _SafeCallFunction<bool>("engine_x64_rwdi.dll", "?IsLoading@ILevel@@QEBA_NXZ", true, this);
	}
	bool LevelDI::IsLoaded() {
		if (IsLoading() || !GamePH::PlayerDI_PH::Get()) {
			ResetLoadTimer();
			return false;
		}

		if (!relyOnTimer && !hasLoaded) {
			relyOnTimer = true;
			loadTimer = Utils::Time::Timer(20000);
			return false;
		}

		if (loadTimer.DidTimePass()) {
			relyOnTimer = false;
			hasLoaded = true;
			return true;
		}

		return false;
	}
	void LevelDI::ResetLoadTimer() {
		relyOnTimer = false;
		hasLoaded = false;
	}
	LPVOID LevelDI::GetViewCamera() {
		return _SafeCallFunction<LPVOID>("engine_x64_rwdi.dll", "?GetViewCamera@ILevel@@QEBAPEAVIBaseCamera@@XZ", nullptr, this);
	}
	float LevelDI::GetTimeDelta() {
		return _SafeCallFunction<float>("gamedll_ph_x64_rwdi.dll", "?GetTimeDelta@IGSObject@@UEBAMXZ", 0.0f, this);
	}
	void LevelDI::SetViewCamera(LPVOID viewCam) {
		_SafeCallFunctionVoid("gamedll_ph_x64_rwdi.dll", "?SetViewCamera@ILevel@@UEAAXPEAVIBaseCamera@@@Z_0", this, viewCam);
	}
	float LevelDI::GetTimePlayed() {
		return _SafeCallFunction<float>("gamedll_ph_x64_rwdi.dll", "?GetTimePlayed@ILevel@@UEBAMXZ", 0.0f, this);
	}
	void LevelDI::ShowUIManager(bool enabled) {
		_SafeCallFunctionVoid("engine_x64_rwdi.dll", "?ShowUIManager@ILevel@@QEAAX_N@Z", this, enabled);
	}
	bool LevelDI::IsTimerFrozen() {
		return _SafeCallFunction<bool>("engine_x64_rwdi.dll", "?IsTimerFrozen@ILevel@@QEBA_NXZ", false, this);
	}
	float LevelDI::TimerGetSpeedUp() {
		return _SafeCallFunction<float>("engine_x64_rwdi.dll", "?TimerGetSpeedUp@ILevel@@QEBAMXZ", -1.0f, this);
	}
	void LevelDI::TimerSetSpeedUp(float timeScale) {
		_SafeCallFunctionVoid("engine_x64_rwdi.dll", "?TimerSetSpeedUp@ILevel@@QEAAXM@Z", this, timeScale);
	}
	TimeWeather::CSystem* LevelDI::GetTimeWeatherSystem() {
		return _SafeCallFunction<TimeWeather::CSystem*>("engine_x64_rwdi.dll", "?GetTimeWeatherSystem@ILevel@@QEBAPEAVCSystem@TimeWeather@@XZ", nullptr, this);
	}

	SAFE_DEPENDENT_OFFSET_GETTER(LevelDI, Engine::CLevel)
	LevelDI* LevelDI::Get() {
		LevelDI* ptr = _SafeGetter<LevelDI>(GetOffset_LevelDI, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_LevelDI);
		if (!ptr)
			ResetLoadTimer();

		return ptr;
	}
}