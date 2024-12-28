#include <pch.h>
#include "..\offsets.h"
#include "CGame.h"
#include "CVideoSettings.h"

namespace Engine {
    static CVideoSettings* GetOffset_CVideoSettings() {
        CGame* pCGame = CGame::Get();
        return pCGame ? pCGame->pCVideoSettings : nullptr;
    }
    CVideoSettings* CVideoSettings::Get() {
        return _SafeGetter<CVideoSettings>(GetOffset_CVideoSettings, "engine_x64_rwdi.dll", false, Offsets::GetVT_CVideoSettings);
    }
}