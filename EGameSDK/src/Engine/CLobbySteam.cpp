#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CLobbySteam.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	CLobbySteam* CLobbySteam::Get() {
		return ClassHelpers::SafeGetter<CLobbySteam>(Offsets::Get_CLobbySteam, true);
	}
}