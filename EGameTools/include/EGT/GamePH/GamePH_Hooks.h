#pragma once
#include <EGSDK\Utils\Hook.h>

namespace EGT::GamePH {
	namespace Hooks {
		extern EGSDK::Utils::Hook::ByteHook<void*> SaveGameCRCBoolCheckHook;
	}
}