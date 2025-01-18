#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\CoPlayerRestrictions.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static CoPlayerRestrictions* pCoPlayerRestrictions = nullptr;

	DWORD64* CoPlayerRestrictions::GetPlayerRestrictionsFlags(DWORD64* flagsOut) {
		return Utils::Memory::SafeCallFunctionOffset<DWORD64*>(Offsets::Get_GetPlayerRestrictionsFlags, nullptr, this, flagsOut);
	}

	static CoPlayerRestrictions* GetOffset_CoPlayerRestrictions() {
		if (!pCoPlayerRestrictions)
			return nullptr;
		if (!*reinterpret_cast<void**>(pCoPlayerRestrictions))
			return nullptr;
		return pCoPlayerRestrictions;
	}
	CoPlayerRestrictions* CoPlayerRestrictions::Get() {
		return ClassHelpers::SafeGetter<CoPlayerRestrictions>(GetOffset_CoPlayerRestrictions, false, false);
	}

	void CoPlayerRestrictions::SetInstance(void* instance) {
		pCoPlayerRestrictions = reinterpret_cast<CoPlayerRestrictions*>(instance);
	}
}