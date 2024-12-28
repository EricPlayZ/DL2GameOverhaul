#include <pch.h>
#include "..\offsets.h"
#include "CLobbySteam.h"

namespace Engine {
	CLobbySteam* CLobbySteam::Get() {
		return _SafeGetter<CLobbySteam>(Offsets::Get_CLobbySteam, "engine_x64_rwdi.dll", true, Offsets::GetVT_CLobbySteam);
	}
}