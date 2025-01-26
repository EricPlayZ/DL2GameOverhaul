#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class TPPCameraDI;

	class EGameSDK_API CoBaseCameraProxy {
	public:
		union {
			ClassHelpers::StaticBuffer<0xD0, TPPCameraDI*> pTPPCameraDI;
		};
	};
}