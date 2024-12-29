#pragma once
#include <EGSDK\Engine\CBaseCamera.h>

namespace EGSDK::GamePH {
	class EGameSDK_API TPPCameraDI : public Engine::CBaseCamera {
	public:
		static TPPCameraDI* Get();
	};
}