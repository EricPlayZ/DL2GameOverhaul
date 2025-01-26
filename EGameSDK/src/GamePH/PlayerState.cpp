#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\PlayerState.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	PlayerState* PlayerState::Get() {
		return ClassHelpers::SafeGetter<PlayerState>(OffsetManager::Get_PlayerState, true);
	}
}