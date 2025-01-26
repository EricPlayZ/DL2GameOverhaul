#pragma once
#include <EGSDK\Exports.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	namespace TimeWeather {
		struct EGameSDK_API ISubsystem {
			union {
				float currentTime;
				ClassHelpers::StaticBuffer<0x4, int> currentWeather;
				ClassHelpers::StaticBuffer<0x10, float> nextTime;
				ClassHelpers::StaticBuffer<0x14, int> nextWeather;
				ClassHelpers::StaticBuffer<0x20, float> deltaBlend;
				ClassHelpers::StaticBuffer<0x24, char> hasFinishedFlag;
				ClassHelpers::StaticBuffer<0x28, float> blendTime;
				ClassHelpers::StaticBuffer<0x2C, float> deltaBlend2;
			};
		};
	}
}