#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CGSObject2.h>
#include <EGSDK\GamePH\LogicalPlayer.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH{
    static LogicalPlayer* GetOffset_LogicalPlayer() {
        Engine::CGSObject2* pCGSObject2 = Engine::CGSObject2::Get();
        return pCGSObject2 ? pCGSObject2->pLogicalPlayer : nullptr;
    }
    LogicalPlayer* LogicalPlayer::Get() {
        return _SafeGetter<LogicalPlayer>(GetOffset_LogicalPlayer, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_LogicalPlayer);
    }
}