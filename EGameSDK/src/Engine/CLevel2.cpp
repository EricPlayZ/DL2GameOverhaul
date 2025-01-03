#include <EGSDK\Engine\CGSObject.h>
#include <EGSDK\Engine\CLevel2.h>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Engine {
    static CLevel2* GetOffset_CLevel2() {
        CGSObject* pCGSObject = CGSObject::Get();
        return pCGSObject ? pCGSObject->pCLevel2 : nullptr;
    }
    CLevel2* CLevel2::Get() {
        return ClassHelpers::SafeGetter<CLevel2>(GetOffset_CLevel2, false);
    }
}