#include <EGSDK\Engine\CGame.h>
#include <EGSDK\Engine\CLobbySteam.h>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Engine {
    static CGame* GetOffset_CGame() {
        CLobbySteam* pCLobbySteam = CLobbySteam::Get();
        return pCLobbySteam ? pCLobbySteam->pCGame : nullptr;
    }
    CGame* CGame::Get() {
        return _SafeGetter<CGame>(GetOffset_CGame, "engine_x64_rwdi.dll", false, nullptr);
    }
}