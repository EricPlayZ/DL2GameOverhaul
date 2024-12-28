#include <pch.h>
#include "CGSObject.h"
#include "CLevel2.h"

namespace Engine {
    static CLevel2* GetOffset_CLevel2() {
        CGSObject* pCGSObject = CGSObject::Get();
        return pCGSObject ? pCGSObject->pCLevel2 : nullptr;
    }
    CLevel2* CLevel2::Get() {
        return _SafeGetter<CLevel2>(GetOffset_CLevel2, "engine_x64_rwdi.dll", false, nullptr);
    }
}