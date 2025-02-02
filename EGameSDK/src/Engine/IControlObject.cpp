#include <EGSDK\Engine\IControlObject.h>
#include <EGSDK\Utils\WinMemory.h>

namespace EGSDK::Engine {
	void IControlObject::SetLocalDir(const Vec3* dir) {
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?SetLocalDir@IControlObject@@QEAAXAEBVvec3@@@Z", this, dir);
	}
}