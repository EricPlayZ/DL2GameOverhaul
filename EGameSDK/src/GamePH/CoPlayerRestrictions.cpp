#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\CoPlayerRestrictions.h>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\WinMemory.h>

namespace EGSDK::GamePH {
	static CoPlayerRestrictions* pCoPlayerRestrictions = nullptr;

	uint64_t* CoPlayerRestrictions::GetPlayerRestrictionsFlags(uint64_t* flagsOut) {
		return Utils::Memory::SafeCallFunctionOffset<uint64_t*>(OffsetManager::Get_GetPlayerRestrictionsFlags, nullptr, this, flagsOut);
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