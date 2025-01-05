#include <Windows.h>
#include <EGSDK\Engine\Engine_Hooks.h>
#include <EGSDK\Core\Core.h>
#include <EGSDK\Offsets.h>
#include <EGSDK\Utils\Hook.h>

namespace EGSDK::Engine {
	namespace Hooks {
#pragma region ReadVideoSettings
		Utils::Hook::MHook<void*, bool(*)(void*, void*, bool), void*, void*, bool> ReadVideoSettingsHook{ "ReadVideoSettings", &Offsets::Get_ReadVideoSettings, [](void* instance, void* file, bool flag1) -> bool {
			if (!Core::rendererAPI) {
				DWORD renderer = *reinterpret_cast<PDWORD>(reinterpret_cast<DWORD64>(instance) + 0x7C);
				Core::rendererAPI = !renderer ? 11 : 12;
			}

			ReadVideoSettingsHook.ExecuteCallbacks(instance, file, flag1);
			return ReadVideoSettingsHook.pOriginal(instance, file, flag1);
		}};
#pragma endregion
	}
}