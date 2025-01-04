#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\GamePH\TimeWeather\CSystem.h>
#include <EGSDK\GamePH\TimeWeather\EWeather.h>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::GamePH {
	namespace TimeWeather {
		void CSystem::SetForcedWeather(int weather) {
			Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetForcedWeather@CSystem@TimeWeather@@QEAAXW4TYPE@EWeather@@VApiDebugAccess@2@@Z", this, weather);
		}
		int CSystem::GetCurrentWeather() {
			return Utils::Memory::SafeCallFunction<int>("engine_x64_rwdi.dll", "?GetCurrentWeather@CSystem@TimeWeather@@QEBA?AW4TYPE@EWeather@@XZ", EWeather::TYPE::Default, this);
		}

		static CSystem* GetOffset_CSystem() {
			LevelDI* pLevelDI = LevelDI::Get();
			return pLevelDI ? pLevelDI->GetTimeWeatherSystem() : nullptr;
		}
		CSystem* CSystem::Get() {
			return ClassHelpers::SafeGetter<CSystem>(GetOffset_CSystem, false, false);
		}
	}
}