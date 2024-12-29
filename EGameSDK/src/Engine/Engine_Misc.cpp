#include <EGSDK\Engine\Engine_Misc.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Engine {
	void AuthenticateDataResultsClear(void* instance) {
		Utils::Memory::_SafeCallFunctionVoid("engine_x64_rwdi.dll", "?Clear@Results@AuthenticateData@@QEAAXXZ", instance);
	}
}
