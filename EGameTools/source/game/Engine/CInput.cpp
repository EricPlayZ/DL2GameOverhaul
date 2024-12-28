#include <pch.h>
#include "..\offsets.h"
#include "CInput.h"

namespace Engine {
	DWORD64 CInput::BlockGameInput() {
		return Utils::Memory::CallVT<2, DWORD64>(this);
	}
	void CInput::UnlockGameInput() {
		Utils::Memory::CallVT<1>(this);
	}

	SafeGetterVT(CInput, "engine_x64_rwdi.dll")
}