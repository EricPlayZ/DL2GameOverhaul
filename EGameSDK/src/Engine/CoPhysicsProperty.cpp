#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\Engine\CoPhysicsProperty.h>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Engine {
    static CoPhysicsProperty* GetOffset_CoPhysicsProperty() {
        GamePH::PlayerDI_PH* pPlayerDI_PH = GamePH::PlayerDI_PH::Get();
        return pPlayerDI_PH ? pPlayerDI_PH->pCoPhysicsProperty : nullptr;
    }
    CoPhysicsProperty* CoPhysicsProperty::Get() {
        return ClassHelpers::SafeGetter<CoPhysicsProperty>(GetOffset_CoPhysicsProperty, false, false);
    }
}