#include <pch.h>
#include "..\offsets.h"
#include "CGSObject.h"
#include "CLevel.h"

namespace Engine {
    static CGSObject* GetOffset_CGSObject() {
        CLevel* pCLevel = CLevel::Get();
        return pCLevel ? pCLevel->pCGSObject : nullptr;
    }
    CGSObject* CGSObject::Get() {
        return _SafeGetter<CGSObject>(GetOffset_CGSObject, "engine_x64_rwdi.dll", false, Offsets::GetVT_CGSObject);
    }
}