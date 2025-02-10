#define DIRECTINPUT_VERSION 0x0800
#include <Windows.h>
#include <dinput.h>
#include <spdlog\spdlog.h>
#include <ImGui\imgui_hotkey.h>
#include <EGSDK\Utils\Hook.h>
#include <EGSDK\Engine\CInput.h>
#include <EGT\Menu\Menu.h>
#include <EGT\Menu\Camera.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace EGT::ImGui_impl {
	namespace Win32 {
		static HWND gHwnd = nullptr;
		static WNDPROC oWndProc = nullptr;

		static LRESULT __stdcall hkWindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
			switch (uMsg) {
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if (Menu::firstTimeRunning.GetValue())
					break;

				for (auto& option : *ImGui::KeyBindOption::GetInstances()) {
					if (option->GetChangesAreDisabled())
						continue;
					if (option->GetKeyBind() == VK_NONE)
						continue;

					if (wParam == option->GetKeyBind()) {
						if (!Menu::menuToggle.GetValue()) {
							option->SetIsKeyPressed(!option->IsKeyDown());
							option->SetIsKeyDown(true);
						}

						if (option->IsToggleableOption() && !ImGui::KeyBindOption::wasAnyHotkeyToggled && !ImGui::isAnyHotkeyBtnClicked && ImGui::timeSinceHotkeyBtnPressed.DidTimePass()) {
							ImGui::KeyBindOption::wasAnyHotkeyToggled = true;
							option->Toggle();
						}
					}
				}
				break;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				for (auto& option : *ImGui::KeyBindOption::GetInstances()) {
					if (wParam == option->GetKeyBind()) {
						if (!Menu::menuToggle.GetValue()) {
							option->SetIsKeyReleased(option->IsKeyDown());
							option->SetIsKeyPressed(false);
							option->SetIsKeyDown(false);
						}

						ImGui::KeyBindOption::wasAnyHotkeyToggled = false;
					}
				}
				break;
			default:
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

		static EGSDK::Utils::Hook::MHook<void*, HRESULT(*)(IDirectInputDevice8*, unsigned long, LPDIDEVICEOBJECTDATA, unsigned long*, unsigned long), IDirectInputDevice8*, unsigned long, LPDIDEVICEOBJECTDATA, unsigned long*, unsigned long> CDIDev_GetDeviceDataHook{ "CDIDev_GetDeviceData", &EGSDK::OffsetManager::Get_CDIDev_GetDeviceData, [](IDirectInputDevice8* device, unsigned long cbObjectData, LPDIDEVICEOBJECTDATA rgdod, unsigned long* pdwInOut, unsigned long dwFlags) -> HRESULT {
			HRESULT result = CDIDev_GetDeviceDataHook.ExecuteOriginal(device, cbObjectData, rgdod, pdwInOut, dwFlags);

			if (!rgdod || !pdwInOut || *pdwInOut == 0)
				return result;

			unsigned int validEvents = 0;
			for (unsigned int i = 0; i < *pdwInOut; i++) {
				if (rgdod[i].dwOfs == DIMOFS_Z) {
					long scrollDelta = rgdod[i].dwData;

					for (auto& option : *ImGui::KeyBindOption::GetInstances()) {
						if (option->GetChangesAreDisabled())
							continue;

						if ((option->GetKeyBind() == VK_MWHEELUP && scrollDelta > 0) ||
							(option->GetKeyBind() == VK_MWHEELDOWN && scrollDelta < 0))
							option->Toggle();
					}

					if (scrollDelta > 0)
						ImGui::KeyBindOption::scrolledMouseWheelUp = true;
					else if (scrollDelta < 0)
						ImGui::KeyBindOption::scrolledMouseWheelDown = true;

					if (Menu::Camera::firstPersonZoomIn.IsKeyDown())
						continue;
				}
				rgdod[validEvents++] = rgdod[i];
			}

			*pdwInOut = validEvents;

			return result;
		} };

		void Init(HWND hwnd) {
			gHwnd = hwnd;
			oWndProc = (WNDPROC)SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)hkWindowProc);
		}
	}
}