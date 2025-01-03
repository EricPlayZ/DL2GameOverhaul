#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	class EGameSDK_API CVideoSettings {
	public:
		union {
			ClassHelpers::buffer<0x7C, float> extraFOV;
		};

		static CVideoSettings* Get();
	};
}