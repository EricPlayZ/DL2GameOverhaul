#include <pch.h>
#include "..\offsets.h"
#include "CGame.h"
#include "CLevel.h"

namespace Engine {
    static CLevel* GetOffset_CLevel() {
        CGame* pCGame = CGame::Get();
        return pCGame ? pCGame->pCLevel : nullptr;
    }
    CLevel* CLevel::Get() {
        return _SafeGetter<CLevel>(GetOffset_CLevel, "engine_x64_rwdi.dll", false, Offsets::GetVT_CLevel);
    }
}