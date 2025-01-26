#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\DayNightCycle.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	void DayNightCycle::SetDaytime(float time) {
		time /= 24;
		time1 = time;
		time2 = time;
		time3 = time;
	}

	DayNightCycle* DayNightCycle::Get() {
		return ClassHelpers::SafeGetter<DayNightCycle>(OffsetManager::Get_DayNightCycle, true);
	}
}