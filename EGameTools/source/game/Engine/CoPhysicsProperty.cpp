#include <pch.h>
#include "..\GamePH\PlayerDI_PH.h"
#include "CoPhysicsProperty.h"

namespace Engine {
    static CoPhysicsProperty* GetOffset_CoPhysicsProperty() {
        GamePH::PlayerDI_PH* pPlayerDI_PH = GamePH::PlayerDI_PH::Get();
        return pPlayerDI_PH ? pPlayerDI_PH->pCoPhysicsProperty : nullptr;
    }
    CoPhysicsProperty* CoPhysicsProperty::Get() {
        return _SafeGetter<CoPhysicsProperty>(GetOffset_CoPhysicsProperty, "engine_x64_rwdi.dll", false, nullptr);
    }
}