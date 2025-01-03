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
        return ClassHelpers::SafeGetter<CVideoSettings>(GetOffset_CVideoSettings, false);
    }
}