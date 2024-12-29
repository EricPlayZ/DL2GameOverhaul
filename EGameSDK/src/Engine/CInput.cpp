#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CInput.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	DWORD64 CInput::BlockGameInput() {
		return Utils::Memory::CallVT<2, DWORD64>(this);
	}
	void CInput::UnlockGameInput() {
		Utils::Memory::CallVT<1>(this);
	}

	CInput* CInput::Get() {
		return _SafeGetter<CInput>(Offsets::Get_CInput, "engine_x64_rwdi.dll", true, Offsets::GetVT_CInput);
	}
}