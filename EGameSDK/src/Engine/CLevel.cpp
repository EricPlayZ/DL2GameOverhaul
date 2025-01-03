#include <EGSDK\Offsets.h>
#include <EGSDK\Engine\CGame.h>
#include <EGSDK\Engine\CLevel.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::Engine {
    static CLevel* GetOffset_CLevel() {
        CGame* pCGame = CGame::Get();
        return pCGame ? pCGame->pCLevel : nullptr;
    }
    CLevel* CLevel::Get() {
        return ClassHelpers::SafeGetter<CLevel>(GetOffset_CLevel, false);
    }
}