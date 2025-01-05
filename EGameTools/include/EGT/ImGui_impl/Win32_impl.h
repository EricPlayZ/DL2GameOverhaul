#pragma once
#include <Windows.h>

namespace EGT::ImGui_impl {
	namespace Win32 {
		extern void ToggleMouseHook(bool value);

		extern void Init(HWND hwnd);
	}
}