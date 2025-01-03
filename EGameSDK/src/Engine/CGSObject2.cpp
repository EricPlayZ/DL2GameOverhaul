#include <EGSDK\Engine\CGSObject2.h>
#include <EGSDK\Engine\CLevel2.h>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Engine {
    static CGSObject2* GetOffset_CGSObject2() {
        CLevel2* pCLevel2 = CLevel2::Get();
        return pCLevel2 ? pCLevel2->pCGSObject2 : nullptr;
    }
    CGSObject2* CGSObject2::Get() {
        return ClassHelpers::SafeGetter<CGSObject2>(GetOffset_CGSObject2, false);
    }
}