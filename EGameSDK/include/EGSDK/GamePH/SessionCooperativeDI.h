#pragma once
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class LocalClientDI;

	class EGameSDK_API SessionCooperativeDI {
	public:
		union {
			DynamicField(SessionCooperativeDI, LocalClientDI*, pLocalClientDI);
		};

		static SessionCooperativeDI* Get();
	};
}