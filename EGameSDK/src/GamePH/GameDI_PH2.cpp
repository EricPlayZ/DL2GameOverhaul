#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\GameDI_PH.h>
#include <EGSDK\GamePH\GameDI_PH2.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static GameDI_PH2* GetOffset_GameDI_PH2() {
		GameDI_PH* pGameDI_PH = GameDI_PH::Get();
		return pGameDI_PH ? reinterpret_cast<GameDI_PH2*>(reinterpret_cast<DWORD64>(pGameDI_PH) + Offsets::Get_gameDI_PH2_offset()) : nullptr;
	}
	GameDI_PH2* GameDI_PH2::Get() {
		return ClassHelpers::SafeGetter<GameDI_PH2>(GetOffset_GameDI_PH2, false);
	}
}