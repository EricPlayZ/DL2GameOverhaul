#pragma once
#include <EGSDK\Vec3.h>

namespace EGSDK::Engine {
	class EGameSDK_API IControlObject {
	public:
		void SetLocalDir(const Vec3* dir);
	};
}