#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace EGT::ImGui_impl {
	namespace Win32 {
		extern void EnableMouseHook();
		extern void DisableMouseHook();

		extern void Init(HWND hwnd);
	}
}