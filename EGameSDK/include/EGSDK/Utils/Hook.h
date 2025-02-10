#pragma once
#include <functional>
#include <set>
#include <unordered_map>
#include <string_view>
#include <stdint.h>
#include <MinHook\MinHook.h>
#include <spdlog\spdlog.h>
#include <EGSDK\Exports.h>
#include <EGSDK\Offsets.h>
#include <EGSDK\Utils\Time.h>

namespace EGSDK::Utils {
	namespace Hook {
		static constexpr uint64_t maxTimeSpentHooking = 90;
		static constexpr uint64_t maxTimeSpentHookingMs = maxTimeSpentHooking * 1000;

		extern EGameSDK_API void HookVT(void* instance, void* pDetour, void** ppOriginal, uint32_t offset);

		class EGameSDK_API HookBase {
		public:
			HookBase(const std::string_view& name, bool hookOnStartup = true);
			~HookBase();
			static std::unordered_map<HMODULE, std::set<HookBase*>>* GetInstances();
			std::string_view GetName();

			virtual bool TryHooking() {
				return false;
			};

			bool IsHooking();
			void SetHooking(bool value);

			bool IsHooked();
			void SetHooked(bool value);

			bool CanHookOnStartup() const;
		protected:
		private:
			std::string_view* name = nullptr;
			volatile long isHooking = 0;
			volatile long isHooked = 0;
			bool hookOnStartup = true;
		};
		template <typename OffsetRetType>
		class ByteHook : public HookBase {
		public:
			ByteHook(const std::string_view& name, OffsetRetType(*pGetOffsetFunc)(), unsigned char* patchBytes, size_t bytesAmount, bool hookOnStartup = true) : HookBase(name, hookOnStartup), pGetOffsetFunc(pGetOffsetFunc), patchBytes(patchBytes), bytesAmount(bytesAmount) {}

			bool TryHooking() override {
				if (IsHooked())
					return true;
				if (IsHooking())
					return true;

				SetHooking(true);
				timeSpentHooking.Reset();

				while (true) {
					if (timeSpentHooking.DidTimePass()) {
						SPDLOG_ERROR("Failed hooking \"{}\" after {} seconds", GetName().data(), maxTimeSpentHooking);
						SetHooking(false);
						return false;
					}
					if (!pGetOffsetFunc || !pGetOffsetFunc())
						continue;

					unsigned long originalProtection = 0;
					unsigned long oldProtection = 0;

					VirtualProtect(pGetOffsetFunc(), bytesAmount, PAGE_EXECUTE_READWRITE, &originalProtection);
					if (!origBytes) {
						origBytes = new uint8_t[bytesAmount];
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
				if (IsHooked() || !pGetOffsetFunc || !pGetOffsetFunc())
					return;

				unsigned long originalProtection = 0;
				unsigned long oldProtection = 0;

				VirtualProtect(pGetOffsetFunc(), bytesAmount, PAGE_EXECUTE_READWRITE, &originalProtection);
				if (!origBytes) {
					origBytes = new uint8_t[bytesAmount];
					memcpy_s(origBytes, bytesAmount, pGetOffsetFunc(), bytesAmount);
				}
				memcpy_s(pGetOffsetFunc(), bytesAmount, patchBytes, bytesAmount);
				VirtualProtect(pGetOffsetFunc(), bytesAmount, originalProtection, &oldProtection);
				SetHooked(true);
			}
			void Disable() {
				if (!IsHooked() || !pGetOffsetFunc || !pGetOffsetFunc())
					return;

				unsigned long originalProtection = 0;
				unsigned long oldProtection = 0;

				VirtualProtect(pGetOffsetFunc(), bytesAmount, PAGE_EXECUTE_READWRITE, &originalProtection);
				memcpy_s(pGetOffsetFunc(), bytesAmount, origBytes, bytesAmount);
				VirtualProtect(pGetOffsetFunc(), bytesAmount, originalProtection, &oldProtection);
				SetHooked(false);
			}
		private:
			OffsetRetType(*pGetOffsetFunc)() = nullptr;
			uint8_t* origBytes = nullptr;
			uint8_t* patchBytes = nullptr;
			size_t bytesAmount = 0;

			Utils::Time::Timer timeSpentHooking{ maxTimeSpentHookingMs };
		};
		template <typename OffsetRetType, typename OrigFuncType, typename... Args>
		class MHook : public HookBase {
		public:
			using CallbackType = std::function<void(Args...)>;

			MHook(const std::string_view& name, OffsetRetType(*pGetOffsetFunc)(), OrigFuncType pDetour, bool hookOnStartup = true) : HookBase(name, hookOnStartup), pGetOffsetFunc(pGetOffsetFunc), pDetour(pDetour) {}

			bool TryHooking() override {
				if (pOriginal)
					return true;
				if (IsHooking())
					return true;

				SetHooking(true);
				timeSpentHooking.Reset();

				while (true) {
					if (timeSpentHooking.DidTimePass()) {
						SPDLOG_ERROR("Failed hooking function \"{}\" after {} seconds", GetName().data(), maxTimeSpentHooking);
						SetHooking(false);
						return false;
					}
					if (!pGetOffsetFunc)
						continue;

					if (!pTarget)
						pTarget = reinterpret_cast<OrigFuncType>(pGetOffsetFunc());
					else if (!pOriginal && MH_CreateHook(pTarget, pDetour, reinterpret_cast<void**>(&pOriginal)) == MH_OK) {
						MH_EnableHook(pTarget);
						SetHooked(true);
						SetHooking(false);
						return true;
					}

					Sleep(10);
				}
			}

			auto ExecuteOriginal(Args... args) {
				return pOriginal(args...);
			}
			auto ExecuteDetour(Args... args) {
				return pDetour(args...);
			}

			void RegisterCallback(CallbackType callback) {
				callbacks.push_back(callback);
			}
			void ExecuteCallbacks(Args... args) {
				for (const auto& callback : callbacks)
					callback(args...);
			}
			auto ExecuteCallbacksWithOriginal(Args... args) {
				ExecuteCallbacks(args...);
				return ExecuteOriginal(args...);
			}
		private:
			OffsetRetType(*pGetOffsetFunc)() = nullptr;
			OrigFuncType pOriginal = nullptr;
			OrigFuncType pTarget = nullptr;
			OrigFuncType pDetour = nullptr;

			Utils::Time::Timer timeSpentHooking{ maxTimeSpentHookingMs };
			std::vector<CallbackType> callbacks{};
		};
		template <typename OffsetRetType, typename OrigFuncType, typename... Args>
		class VTHook : public HookBase {
		public:
			using CallbackType = std::function<void(Args...)>;

			VTHook(const std::string_view& name, OffsetRetType(*pGetOffsetFunc)(), OrigFuncType pDetour, bool hookOnStartup = true) : HookBase(name, hookOnStartup), pGetOffsetFunc(pGetOffsetFunc), pDetour(pDetour) {}

			bool TryHooking() override {
				if (pOriginal)
					return true;
				if (IsHooking())
					return true;

				SetHooking(true);
				timeSpentHooking.Reset();

				while (true) {
					if (timeSpentHooking.DidTimePass()) {
						SPDLOG_ERROR("Failed hooking function \"{}\" after {} seconds", GetName().data(), maxTimeSpentHooking);
						SetHooking(false);
						return false;
					}
					if (!pGetOffsetFunc)
						continue;

					if (!pTarget)
						pTarget = reinterpret_cast<OrigFuncType>(pGetOffsetFunc());
					else if (!pOriginal) {
						uint32_t offset = OffsetManager::GetOffset(GetName().data());
						HookVT(pTarget, pDetour, reinterpret_cast<void**>(&pOriginal), offset);
						if (pOriginal) {
							SetHooked(true);
							SetHooking(false);
							return true;
						}
					}

					Sleep(10);
				}
			}

			auto ExecuteOriginal(Args... args) {
				return pOriginal(args...);
			}
			auto ExecuteDetour(Args... args) {
				return pDetour(args...);
			}

			void RegisterCallback(CallbackType callback) {
				callbacks.push_back(callback);
			}
			void ExecuteCallbacks(Args... args) {
				for (const auto& callback : callbacks)
					callback(args...);
			}
			auto ExecuteCallbacksWithOriginal(Args... args) {
				ExecuteCallbacks(args...);
				return ExecuteOriginal(args...);
			}
		private:
			OffsetRetType(*pGetOffsetFunc)() = nullptr;
			OrigFuncType pOriginal = nullptr;
			OrigFuncType pTarget = nullptr;
			OrigFuncType pDetour = nullptr;

			Utils::Time::Timer timeSpentHooking{ maxTimeSpentHookingMs };
			std::vector<CallbackType> callbacks{};
		};
	}
}