#include <pch.h>

namespace Engine {
	void AuthenticateDataResultsClear(LPVOID instance) {
		_SafeCallFunctionVoid("engine_x64_rwdi.dll", "?Clear@Results@AuthenticateData@@QEAAXXZ", instance);
	}
}
