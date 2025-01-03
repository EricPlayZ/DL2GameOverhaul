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
        return ClassHelpers::SafeGetter<LogicalPlayer>(GetOffset_LogicalPlayer, false);
    }
}