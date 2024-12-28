#include <pch.h>
#include "..\offsets.h"
#include "GameDI_PH.h"
#include "GameDI_PH2.h"

namespace GamePH {
	static GameDI_PH2* GetOffset_GameDI_PH2() {
		GameDI_PH* pGameDI_PH = GameDI_PH::Get();
		return pGameDI_PH ? reinterpret_cast<GameDI_PH2*>(reinterpret_cast<DWORD64>(pGameDI_PH) + Offsets::Get_gameDI_PH2_offset()) : nullptr;
	}
	SafeGetterDepCustom(GameDI_PH2, GetOffset_GameDI_PH2, false, "gamedll_ph_x64_rwdi.dll")
}