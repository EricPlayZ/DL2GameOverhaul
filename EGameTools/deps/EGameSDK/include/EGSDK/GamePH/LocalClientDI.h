#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class PlayerDI_PH;

	class EGameSDK_API LocalClientDI {
	public:
		union {
			buffer<0x90, PlayerDI_PH*> pPlayerDI_PH;
		};

		static LocalClientDI* Get();
	};
}