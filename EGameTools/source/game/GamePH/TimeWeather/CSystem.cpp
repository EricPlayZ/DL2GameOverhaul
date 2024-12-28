#include <pch.h>
#include "..\LevelDI.h"
#include "CSystem.h"
#include "EWeather.h"

namespace GamePH {
	namespace TimeWeather {
		void CSystem::SetForcedWeather(int weather) {
			_SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetForcedWeather@CSystem@TimeWeather@@QEAAXW4TYPE@EWeather@@VApiDebugAccess@2@@Z", this, weather);
		}
		int CSystem::GetCurrentWeather() {
			return _SafeCallFunction<int>("engine_x64_rwdi.dll", "?GetCurrentWeather@CSystem@TimeWeather@@QEBA?AW4TYPE@EWeather@@XZ", EWeather::TYPE::Default, this);
		}

		static CSystem* GetOffset_CSystem() {
			LevelDI* pLevelDI = LevelDI::Get();
			return pLevelDI ? pLevelDI->GetTimeWeatherSystem() : nullptr;
		}
		SafeGetterDepCustom(CSystem, GetOffset_CSystem, false, "gamedll_ph_x64_rwdi.dll")
	}
}