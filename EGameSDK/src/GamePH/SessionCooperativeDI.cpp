#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\GameDI_PH.h>
#include <EGSDK\GamePH\SessionCooperativeDI.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
    static SessionCooperativeDI* GetOffset_SessionCooperativeDI() {
        GameDI_PH* pGameDI_PH = GameDI_PH::Get();
        return pGameDI_PH ? pGameDI_PH->pSessionCooperativeDI : nullptr;
    }
    SessionCooperativeDI* SessionCooperativeDI::Get() {
        return _SafeGetter<SessionCooperativeDI>(GetOffset_SessionCooperativeDI, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_SessionCooperativeDI);
    }
}