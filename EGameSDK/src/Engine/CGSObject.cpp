#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CGSObject.h>
#include <EGSDK\Engine\CLevel.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
    static CGSObject* GetOffset_CGSObject() {
        CLevel* pCLevel = CLevel::Get();
        return pCLevel ? pCLevel->pCGSObject : nullptr;
    }
    CGSObject* CGSObject::Get() {
        return _SafeGetter<CGSObject>(GetOffset_CGSObject, "engine_x64_rwdi.dll", false, Offsets::GetVT_CGSObject);
    }
}