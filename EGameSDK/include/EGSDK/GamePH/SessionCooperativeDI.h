#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class LocalClientDI;

	class EGameSDK_API SessionCooperativeDI {
	public:
		union {
			ClassHelpers::buffer<0xE08, LocalClientDI*> pLocalClientDI;
		};

		static SessionCooperativeDI* Get();
	};
}