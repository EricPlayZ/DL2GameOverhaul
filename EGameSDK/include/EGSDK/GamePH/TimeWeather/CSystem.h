#pragma once
#include <EGSDK\GamePH\TimeWeather\ISubsystem.h>
#include <EGSDK\Exports.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	namespace TimeWeather {
		class EGameSDK_API CSystem {
		public:
			union {
				ClassHelpers::StaticBuffer<0x10, float> blendTime;
				ClassHelpers::StaticBuffer<0x14, float> blendTime2;
				ClassHelpers::StaticBuffer<0x70, ISubsystem*> currentSubSystem;
				ClassHelpers::StaticBuffer<0x78, ISubsystem*> nextSubSystem;
				ClassHelpers::StaticBuffer<0x80, ISubsystem*> lastSubSystem;
			};

			void SetForcedWeather(int weather);
			void ClearForcedWeather();
			int GetCurrentWeather();

			void ReloadSubsystems();
			void RequestTimeWeatherInterpolation(int weather, float a3, float a4);
			void FinishTimeWeatherInterpolation();
			bool IsFullyBlended();

			static CSystem* Get();
		};
	}
}