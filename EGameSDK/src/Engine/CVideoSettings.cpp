#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CGame.h>
#include <EGSDK\Engine\CVideoSettings.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
    static CVideoSettings* GetOffset_CVideoSettings() {
        CGame* pCGame = CGame::Get();
        return pCGame ? pCGame->pCVideoSettings : nullptr;
    }
    CVideoSettings* CVideoSettings::Get() {
        return _SafeGetter<CVideoSettings>(GetOffset_CVideoSettings, "engine_x64_rwdi.dll", false, Offsets::GetVT_CVideoSettings);
    }
}