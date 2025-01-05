#pragma once
#include <Windows.h>
#include <functional>
#include <set>
#include <unordered_map>
#include <string_view>
#include <MinHook\MinHook.h>
#include <spdlog\spdlog.h>
#include <EGSDK\Exports.h>
#include <EGSDK\Utils\Time.h>

namespace EGSDK::Utils {
	namespace Hook {
		extern EGameSDK_API void HookVT(void* instance, void* pDetour, void** ppOriginal, const DWORD offset);

		class EGameSDK_API BreakpointHook {
		private:
			PDWORD64 m_addr;
			BYTE m_originalBytes;
			void (*m_handler)(PEXCEPTION_POINTERS);
			DWORD m_originalProtection;

			static long WINAPI OnException(PEXCEPTION_POINTERS info);
		public:
			BreakpointHook(PDWORD64 addr, void (*handler)(PEXCEPTION_POINTERS));
			void Enable();
			void Disable();
			~BreakpointHook();
		};

		class EGameSDK_API HookBase {
		public:
			HookBase(const std::string_view& name);
			~HookBase();
			static std::unordered_map<HMODULE, std::set<HookBase*>>* GetInstances();
			std::string_view GetName() { return *name; };

			virtual bool HookLoop() { return false; };

			bool IsHooking();
			void SetHooking(bool value);

			bool IsHooked();
			void SetHooked(bool value);
		private:
			std::string_view* name = nullptr;
			volatile LONG isHooking = 0;
			volatile LONG isHooked = 0;
		};
		template <typename GetTargetOffsetRetType>
		class ByteHook : HookBase {
		public:
			ByteHook(const std::string_view& name, GetTargetOffsetRetType(*pGetOffsetFunc)(), unsigned char* patchBytes, size_t bytesAmount) : HookBase(name), pGetOffsetFunc(pGetOffsetFunc), patchBytes(patchBytes), bytesAmount(bytesAmount) {}

			bool HookLoop() override {
				if (IsHooked())
					return true;
				if (IsHooking())
					return true;

				SetHooking(true);
				timeSpentHooking = Utils::Time::Timer(120000);

				while (true) {
					if (timeSpentHooking.DidTimePass()) {
						SPDLOG_ERROR("Failed hooking \"{}\" after 120 seconds", GetName().data());
						SetHooking(false);
						return false;
					}
					if (!pGetOffsetFunc || !pGetOffsetFunc())
						continue;

					DWORD originalProtection = 0;
					DWORD oldProtection = 0;

					VirtualProtect(pGetOffsetFunc(), bytesAmount, PAGE_EXECUTE_READWRITE, &originalProtection);
					if (!origBytes) {
						origBytes = new unsigned char[bytesAmount];
						memcpy_s(origBytes, bytesAmount, pGetOffsetFunc(), bytesAmount);
					}
					memcpy_s(pGetOffsetFunc(), bytesAmount, patchBytes, bytesAmount);
					VirtualProtect(pGetOffsetFunc(), bytesAmount, originalProtection, &oldProtection);
					SetHooked(true);
					SetHooking(false);
					return true;

					Sleep(10);
				}
			}

			void Enable() {
				if (IsHooked())
					return;

				DWORD originalProtection = 0;
				DWORD oldProtection = 0;

				VirtualProtect(pGetOffsetFunc(), bytesAmount, PAGE_EXECUTE_READWRITE, &originalProtection);
				if (!origBytes) {
					origBytes = new unsigned char[bytesAmount];
					memcpy_s(origBytes, bytesAmount, pGetOffsetFunc(), bytesAmount);
				}
				memcpy_s(pGetOffsetFunc(), bytesAmount, patchBytes, bytesAmount);
				VirtualProtect(pGetOffsetFunc(), bytesAmount, originalProtection, &oldProtection);
				SetHooked(true);
			}
			void Disable() {
				if (!IsHooked())
					return;

				DWORD originalProtection = 0;
				DWORD oldProtection = 0;

				VirtualProtect(pGetOffsetFunc(), bytesAmount, PAGE_EXECUTE_READWRITE, &originalProtection);
				memcpy_s(pGetOffsetFunc(), bytesAmount, origBytes, bytesAmount);
				VirtualProtect(pGetOffsetFunc(), bytesAmount, originalProtection, &oldProtection);
				SetHooked(false);
			}
		private:
			GetTargetOffsetRetType(*pGetOffsetFunc)() = nullptr;
			unsigned char* origBytes = nullptr;
			unsigned char* patchBytes = nullptr;
			size_t bytesAmount = 0;

			Utils::Time::Timer timeSpentHooking{ 120000 };
		};
		template <typename GetTargetOffsetRetType, typename OrigType, typename... Args>
		class MHook : HookBase {
		public:
			using CallbackType = std::function<void(Args...)>;

			MHook(const std::string_view& name, GetTargetOffsetRetType(*pGetOffsetFunc)(), OrigType pDetour) : HookBase(name), pGetOffsetFunc(pGetOffsetFunc), pDetour(pDetour) {}

			bool HookLoop() override {
				if (pOriginal)
					return true;
				if (IsHooking())
					return true;

				SetHooking(true);
				timeSpentHooking = Utils::Time::Timer(120000);

				while (true) {
					if (timeSpentHooking.DidTimePass()) {
						SPDLOG_ERROR("Failed hooking function \"{}\" after 120 seconds", GetName().data());
						SetHooking(false);
						return false;
					}
					if (!pGetOffsetFunc)
						continue;

					if (!pTarget)
						pTarget = reinterpret_cast<OrigType>(pGetOffsetFunc());
					else if (!pOriginal && MH_CreateHook(pTarget, pDetour, reinterpret_cast<void**>(&pOriginal)) == MH_OK) {
						MH_EnableHook(pTarget);
						SetHooked(true);
						SetHooking(false);
						return true;
					}

					Sleep(10);
				}
			}

			void RegisterCallback(CallbackType callback) {
				callbacks.push_back(callback);
			}
			void ExecuteCallbacks(Args... args) {
				for (const auto& callback : callbacks)
					callback(args...);
			}

			OrigType pOriginal = nullptr;
			OrigType pTarget = nullptr;
		private:
			GetTargetOffsetRetType(*pGetOffsetFunc)() = nullptr;
			OrigType pDetour = nullptr;

			Utils::Time::Timer timeSpentHooking{ 120000 };
			std::vector<CallbackType> callbacks;
		};
		template <typename GetTargetOffsetRetType, typename OrigType, typename... Args>
		class VTHook : HookBase {
		public:
			using CallbackType = std::function<void(Args...)>;

			VTHook(const std::string_view& name, GetTargetOffsetRetType(*pGetOffsetFunc)(), OrigType pDetour, DWORD offset) : HookBase(name), pGetOffsetFunc(pGetOffsetFunc), pDetour(pDetour), offset(offset) {}

			bool HookLoop() override {
				if (pOriginal)
					return true;
				if (IsHooking())
					return true;

				SetHooking(true);
				timeSpentHooking = Utils::Time::Timer(120000);

				while (true) {
					if (timeSpentHooking.DidTimePass()) {
						SPDLOG_ERROR("Failed hooking function \"{}\" after 120 seconds", GetName().data());
						SetHooking(false);
						return false;
					}
					if (!pGetOffsetFunc)
						continue;

					if (!pInstance)
						pInstance = pGetOffsetFunc();
					else if (!pOriginal) {
						HookVT(pInstance, pDetour, reinterpret_cast<void**>(&pOriginal), offset);
						if (pOriginal) {
							SetHooked(true);
							SetHooking(false);
							return true;
						}
					}

					Sleep(10);
				}
			}

			void RegisterCallback(CallbackType callback) {
				callbacks.push_back(callback);
			}
			void ExecuteCallbacks(Args... args) {
				for (const auto& callback : callbacks)
					callback(args...);
			}

			OrigType pOriginal = nullptr;
		private:
			GetTargetOffsetRetType(*pGetOffsetFunc)() = nullptr;
			void* pInstance = nullptr;
			OrigType pDetour = nullptr;

			Utils::Time::Timer timeSpentHooking{ 120000 };
			std::vector<CallbackType> callbacks;

			DWORD offset = 0x0;
		};
	}
}