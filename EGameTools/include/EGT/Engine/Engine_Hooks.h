#pragma once
#include <string>
#include <vector>
#include <EGSDK\Vector3.h>

namespace EGT::Engine {
	namespace Hooks {
		extern bool switchedFreeCamByGamePause;
		extern EGSDK::Vector3 freeCamPosBeforeGamePause;
		extern int mountDataPaksRanWith8Count;
	}
}