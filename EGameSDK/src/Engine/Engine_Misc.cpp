#include <EGSDK\Engine\Engine_Misc.h>
#include <EGSDK\Utils\Memory.h>
#include <EGSDK\Utils\WinMemory.h>

namespace EGSDK::Engine {
	void AuthenticateDataResultsClear(void* instance) {
		Utils::Memory::SafeCallFunctionVoid("engine_x64_rwdi.dll", "?Clear@Results@AuthenticateData@@QEAAXXZ", instance);
	}
}
