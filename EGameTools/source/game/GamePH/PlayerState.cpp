#include <pch.h>
#include "..\offsets.h"
#include "PlayerState.h"

namespace GamePH {
	PlayerState* PlayerState::Get() {
		return _SafeGetter<PlayerState>(Offsets::Get_PlayerState, "gamedll_ph_x64_rwdi.dll", true, Offsets::GetVT_PlayerState);
	}
}