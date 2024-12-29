#pragma once
#include <EGSDK\Utils\Hook.h>

namespace EGSDK::Engine {
	namespace Hooks {
		extern EGameSDK_API Utils::Hook::MHook<void*, bool(*)(void*, void*, bool), void*, void*, bool> ReadVideoSettingsHook;
	}
}