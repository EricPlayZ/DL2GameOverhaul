#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\LocalClientDI.h>
#include <EGSDK\GamePH\SessionCooperativeDI.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
    static LocalClientDI* GetOffset_LocalClientDI() {
        SessionCooperativeDI* pSessionCooperativeDI = SessionCooperativeDI::Get();
        return pSessionCooperativeDI ? pSessionCooperativeDI->pLocalClientDI : nullptr;
    }
    LocalClientDI* LocalClientDI::Get() {
        return ClassHelpers::SafeGetter<LocalClientDI>(GetOffset_LocalClientDI, false);
    }
}