#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CGame.h>
#include <EGSDK\GamePH\GameDI_PH.h>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::GamePH {
	float GameDI_PH::GetGameTimeDelta() {
		return Utils::Memory::_SafeCallFunction<int>("engine_x64_rwdi.dll", "?GetGameTimeDelta@IGame@@QEBAMXZ", -1.0f, this);
	}
	void GameDI_PH::TogglePhotoMode(bool doNothing, bool setAsOptionalCamera) {
		Utils::Memory::_SafeCallFunctionOffsetVoid(Offsets::Get_TogglePhotoMode2, this, doNothing, setAsOptionalCamera);
	}

	static GameDI_PH* GetOffset_GameDI_PH() {
		Engine::CGame* pCGame = Engine::CGame::Get();
		return pCGame ? pCGame->pGameDI_PH : nullptr;
	}
	GameDI_PH* GameDI_PH::Get() {
		return _SafeGetter<GameDI_PH>(GetOffset_GameDI_PH, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_GameDI_PH);
	}
}