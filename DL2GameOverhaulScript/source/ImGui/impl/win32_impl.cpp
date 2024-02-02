﻿#include <Hotkey.h>
#include <Windows.h>
#include <backends\imgui_impl_win32.h>
#include <imgui.h>
#include "..\..\core.h"
#include "..\..\game_classes.h"
#include "..\..\kiero.h"
#include "..\..\menu\menu.h"
#include "..\..\sigscan\offsets.h"
#include "..\config\config.h"
#include "win32_impl.h"

static WNDPROC oWndProc = NULL;
static bool toggledMenu = false;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall hkWindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	switch (uMsg) {
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (ImGui::isAnyHotkeyBtnPressed || !ImGui::timeSinceHotkeyBtnPressed.GetTimePassed() || KeyBindOption::wasAnyKeyPressed)
			break;

		for (auto& option : *KeyBindOption::GetInstances()) {
			if (option->GetChangesAreDisabled())
				continue;
			if (wParam == option->GetKeyBind()) {
				KeyBindOption::wasAnyKeyPressed = true;
				option->Toggle();
			}
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (!KeyBindOption::wasAnyKeyPressed)
			break;

		for (auto& option : *KeyBindOption::GetInstances()) {
			if (wParam == option->GetKeyBind())
				KeyBindOption::wasAnyKeyPressed = false;
		}
		break;
	}

	Engine::CInput* pCInput = Engine::CInput::Get();
	if (!pCInput)
		return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);

	ImGui::GetIO().MouseDrawCursor = Menu::menuToggle.GetValue();
	ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);

	if (Menu::menuToggle.GetValue()) {
		if (!toggledMenu)
			pCInput->BlockGameInput();

		toggledMenu = true;
		return true;
	} else if (toggledMenu) {
		toggledMenu = false;
		pCInput->UnlockGameInput();
	}

	return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}

void impl::win32::init(HWND hwnd) {
	oWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)hkWindowProc);
}