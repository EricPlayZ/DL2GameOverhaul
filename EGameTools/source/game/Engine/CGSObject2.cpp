#include <pch.h>
#include "CGSObject2.h"
#include "CLevel2.h"

namespace Engine {
    static CGSObject2* GetOffset_CGSObject2() {
        CLevel2* pCLevel2 = CLevel2::Get();
        return pCLevel2 ? pCLevel2->pCGSObject2 : nullptr;
    }
    CGSObject2* CGSObject2::Get() {
        return _SafeGetter<CGSObject2>(GetOffset_CGSObject2, "engine_x64_rwdi.dll", false, nullptr);
    }
}