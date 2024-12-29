#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class EGameSDK_API DayNightCycle {
	public:
		union {
			buffer<0x10, float> time1;
			buffer<0x20, float> time2;
			buffer<0x5C, float> time3;
		};

		void SetDaytime(float time);

		static DayNightCycle* Get();
	};
}