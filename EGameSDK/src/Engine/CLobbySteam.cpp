#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CLobbySteam.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
	CLobbySteam* CLobbySteam::Get() {
		return _SafeGetter<CLobbySteam>(Offsets::Get_CLobbySteam, "engine_x64_rwdi.dll", true, Offsets::GetVT_CLobbySteam);
	}
}