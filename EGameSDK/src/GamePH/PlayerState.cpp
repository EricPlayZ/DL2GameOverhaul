#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\PlayerState.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	PlayerState* PlayerState::Get() {
		return _SafeGetter<PlayerState>(Offsets::Get_PlayerState, "gamedll_ph_x64_rwdi.dll", true, Offsets::GetVT_PlayerState);
	}
}