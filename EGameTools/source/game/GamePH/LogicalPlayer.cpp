#include <pch.h>
#include "..\Engine\CGSObject2.h"
#include "..\offsets.h"
#include "LogicalPlayer.h"

namespace GamePH {
    static LogicalPlayer* GetOffset_LogicalPlayer() {
        Engine::CGSObject2* pCGSObject2 = Engine::CGSObject2::Get();
        return pCGSObject2 ? pCGSObject2->pLogicalPlayer : nullptr;
    }
    LogicalPlayer* LogicalPlayer::Get() {
        return _SafeGetter<LogicalPlayer>(GetOffset_LogicalPlayer, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_LogicalPlayer);
    }
}