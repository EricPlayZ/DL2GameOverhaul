#pragma once
#include <EGSDK\Exports.h>

namespace EGSDK::GamePH {
	namespace TimeWeather {
		class EGameSDK_API CSystem {
		public:
			void SetForcedWeather(int weather);
			int GetCurrentWeather();

			static CSystem* Get();
		};
	}
}