#include <pch.h>
#include "..\offsets.h"
#include "DayNightCycle.h"

namespace GamePH {
	void DayNightCycle::SetDaytime(float time) {
		time /= 24;
		time1 = time;
		time2 = time;
		time3 = time;
	}

	SafeGetterVT(DayNightCycle, "gamedll_ph_x64_rwdi.dll")
}