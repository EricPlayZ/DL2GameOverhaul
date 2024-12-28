#include <pch.h>
#include "CGame.h"
#include "CLobbySteam.h"

namespace Engine {
    static CGame* GetOffset_CGame() {
        CLobbySteam* pCLobbySteam = CLobbySteam::Get();
        return pCLobbySteam ? pCLobbySteam->pCGame : nullptr;
    }
    CGame* CGame::Get() {
        return _SafeGetter<CGame>(GetOffset_CGame, "engine_x64_rwdi.dll", false, nullptr);
    }
}