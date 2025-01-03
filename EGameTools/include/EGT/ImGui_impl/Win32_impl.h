#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace EGT::ImGui_impl {
	namespace Win32 {
		extern void ToggleMouseHook(bool value);

		extern void Init(HWND hwnd);
	}
}