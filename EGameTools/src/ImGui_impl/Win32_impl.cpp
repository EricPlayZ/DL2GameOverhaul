#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <spdlog\spdlog.h>
#include <ImGui\imgui_hotkey.h>
#include <EGSDK\Engine\CInput.h>
#include <EGT\Menu\Menu.h>
#include <EGT\Menu\Debug.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace EGT::ImGui_impl {
	namespace Win32 {
		static HWND gHwnd = nullptr;
		static WNDPROC oWndProc = nullptr;
		static HHOOK oMouseProc = nullptr;

		static LRESULT __stdcall hkWindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
			switch (uMsg) {
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if (Menu::firstTimeRunning.GetValue() || ImGui::isAnyHotkeyBtnPressed || !ImGui::timeSinceHotkeyBtnPressed.DidTimePass() || ImGui::KeyBindOption::wasAnyKeyPressed)
					break;

				for (auto& option : *ImGui::KeyBindOption::GetInstances()) {
					if (option->GetChangesAreDisabled())
						continue;
					if (option->GetKeyBind() == VK_NONE)
						continue;

					if (wParam == option->GetKeyBind()) {
						ImGui::KeyBindOption::wasAnyKeyPressed = true;
						option->Toggle();
					}
				}
				break;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				if (!ImGui::KeyBindOption::wasAnyKeyPressed)
					break;

				for (auto& option : *ImGui::KeyBindOption::GetInstances()) {
					if (wParam == option->GetKeyBind())
						ImGui::KeyBindOption::wasAnyKeyPressed = false;
				}
				break;
			}

			EGSDK::Engine::CInput* pCInput = EGSDK::Engine::CInput::Get();
			if (!pCInput)
				return CallWindowProcA(oWndProc, hwnd, uMsg, wParam, lParam);

			ImGui::GetIO().MouseDrawCursor = !Menu::hasSeenChangelog.GetValue() || Menu::firstTimeRunning.GetValue() || Menu::menuToggle.GetValue();
			ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);

			if (!Menu::hasSeenChangelog.GetValue() || Menu::firstTimeRunning.GetValue() || Menu::menuToggle.GetValue()) {
				pCInput->BlockGameInput();

				if (Menu::menuToggle.GetValue())
					Menu::menuToggle.SetPrevValue(true);
			} else if (!Menu::hasSeenChangelog.GetPrevValue() || Menu::firstTimeRunning.GetPrevValue() || Menu::menuToggle.GetPrevValue()) {
				if (!Menu::hasSeenChangelog.GetPrevValue())
					Menu::hasSeenChangelog.SetPrevValue(true);
				else if (Menu::firstTimeRunning.GetPrevValue())
					Menu::firstTimeRunning.SetPrevValue(false);
				else if (Menu::menuToggle.GetPrevValue())
					Menu::menuToggle.SetPrevValue(false);
				pCInput->UnlockGameInput();
			}

			return CallWindowProcA(oWndProc, hwnd, uMsg, wParam, lParam);
		}

		static LRESULT CALLBACK hkMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
			if (nCode != HC_ACTION)
				return CallNextHookEx(oMouseProc, nCode, wParam, lParam);
			if (GetForegroundWindow() != gHwnd)
				return CallNextHookEx(oMouseProc, nCode, wParam, lParam);

			switch (wParam) {
			case WM_MOUSEWHEEL:
			case WM_MOUSEHWHEEL:
			{
				MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;
				if (pMouseStruct == nullptr)
					break;

				if (GET_WHEEL_DELTA_WPARAM(pMouseStruct->mouseData)) {
					if (Menu::firstTimeRunning.GetValue())
						break;
					for (auto& option : *ImGui::KeyBindOption::GetInstances()) {
						if (option->GetChangesAreDisabled())
							continue;

						if ((option->GetKeyBind() == VK_MWHEELUP && GET_WHEEL_DELTA_WPARAM(pMouseStruct->mouseData) > 0) ||
							(option->GetKeyBind() == VK_MWHEELDOWN && GET_WHEEL_DELTA_WPARAM(pMouseStruct->mouseData) < 0))
							option->Toggle();
					}

					if (GET_WHEEL_DELTA_WPARAM(pMouseStruct->mouseData) > 0)
						ImGui::KeyBindOption::scrolledMouseWheelUp = true;
					else if (GET_WHEEL_DELTA_WPARAM(pMouseStruct->mouseData) < 0)
						ImGui::KeyBindOption::scrolledMouseWheelDown = true;
				}
				break;
			}
			}

			return CallNextHookEx(oMouseProc, nCode, wParam, lParam);
		}
		static void MouseHkMsgLoop() {
			MSG msg{};
			while (oMouseProc) {
				if (oMouseProc && GetMessageA(&msg, NULL, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessageA(&msg);
				}
			}
		}

		static void EnableMouseHook() {
			if (oMouseProc)
				return;

			std::thread([]() {
				if (oMouseProc)
					return;

				oMouseProc = SetWindowsHookExA(WH_MOUSE_LL, hkMouseProc, GetModuleHandleA(nullptr), 0);
				if (!oMouseProc) {
					SPDLOG_ERROR("Failed to enable low level mouse hook; mouse input-related functions (such as FreeCam speed changing through the scrollwheel) may not work");
					return;
				}

				MouseHkMsgLoop();
			}).detach();
		}
		static void DisableMouseHook() {
			if (!oMouseProc)
				return;

			UnhookWindowsHookEx(oMouseProc);
			oMouseProc = nullptr;
		}
		void ToggleMouseHook(bool disableLowLevelMouseHook) {
			if (disableLowLevelMouseHook)
				DisableMouseHook();
			else
				EnableMouseHook();
		}

		void Init(HWND hwnd) {
			gHwnd = hwnd;
			oWndProc = (WNDPROC)SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)hkWindowProc);

			if (!Menu::Debug::disableLowLevelMouseHook)
				EnableMouseHook();
		}
	}
}