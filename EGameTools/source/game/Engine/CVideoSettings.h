#pragma once
#include "..\ClassHelpers.h"

namespace Engine {
	class CVideoSettings {
	public:
		union {
			buffer<0x7C, float> extraFOV;
		};

		static CVideoSettings* Get();
	};
}