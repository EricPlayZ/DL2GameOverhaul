#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CInput.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	void CInput::UnlockGameInput() {
		Utils::Memory::CallVT<1>(this);
	}
	uint64_t CInput::BlockGameInput() {
		return Utils::Memory::CallVT<2, uint64_t>(this);
	}

	CInput* CInput::Get() {
		return ClassHelpers::SafeGetter<CInput>(OffsetManager::Get_CInput, true);
	}
}