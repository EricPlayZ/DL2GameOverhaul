#include <pch.h>
#include "..\offsets.h"
#include "LocalClientDI.h"
#include "SessionCooperativeDI.h"

namespace GamePH {
    static LocalClientDI* GetOffset_LocalClientDI() {
        SessionCooperativeDI* pSessionCooperativeDI = SessionCooperativeDI::Get();
        return pSessionCooperativeDI ? pSessionCooperativeDI->pLocalClientDI : nullptr;
    }
    LocalClientDI* LocalClientDI::Get() {
        return _SafeGetter<LocalClientDI>(GetOffset_LocalClientDI, "gamedll_ph_x64_rwdi.dll", false, Offsets::GetVT_LocalClientDI);
    }
}