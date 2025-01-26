#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class PlayerDI_PH;

	class EGameSDK_API LocalClientDI {
	public:
		union {
			DynamicField(LocalClientDI, PlayerDI_PH*, pPlayerDI_PH);
		};

		static LocalClientDI* Get();
	};
}