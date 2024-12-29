#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
#include <string>
#include <string_view>
#include <vector>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace Memory {
		static const BYTE SigScanWildCard = 0xAA;
		static const std::string_view SigScanWildCardStr = "AA";

		class SafeExecution {
		private:
			static int fail(unsigned int code, struct _EXCEPTION_POINTERS* ep);
		public:
			template<typename T = void*, typename R = T, typename... Args>
			static T Execute(uint64_t ptr, R ret, Args... args) {
				__try {
					return reinterpret_cast<T(__stdcall*)(Args...)>(ptr)(args...);
				} __except (fail(GetExceptionCode(), GetExceptionInformation())) {
					return ret;
				}
			}

			template<typename... Args>
			static void ExecuteVoid(uint64_t ptr, Args... args) {
				__try {
					return reinterpret_cast<void(__stdcall*)(Args...)>(ptr)(args...);
				} __except (fail(GetExceptionCode(), GetExceptionInformation())) {

				}
			}
		};

		extern EGameSDK_API const MODULEINFO GetModuleInfo(const char* szModule);
		extern EGameSDK_API const FARPROC GetProcAddr(const std::string_view& module, const std::string_view& funcName);

		extern EGameSDK_API const bool IsAddressValidMod(const DWORD64 ptr, const char* moduleName);

		extern EGameSDK_API std::string BytesToIDAPattern(BYTE* bytes, size_t size);
		extern EGameSDK_API std::string ConvertSigToScannerSig(const char* pattern, int* offsetToAddrInSig = nullptr);

		extern EGameSDK_API DWORD64 CalcTargetAddrOfRelInst(DWORD64 addrOfInst, size_t opSize);
		extern EGameSDK_API std::vector<DWORD64> GetXrefsTo(DWORD64 address, DWORD64 start, size_t size);

#pragma region Templates
		template<typename ptrT> bool IsValidPtr(ptrT ptr) {
			__try {
				return !IsBadReadPtr(reinterpret_cast<void*>(ptr), sizeof(void*));
			} __except(EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
		}
		template<typename ptrT = void*> bool IsValidPtrMod(ptrT ptr, const char* moduleName, const bool checkForVT = true) {
			return IsValidPtr<ptrT>(ptr) && IsAddressValidMod(checkForVT ? *(PDWORD64)(ptr) : (DWORD64)(ptr), moduleName);
		}

		template <std::size_t Index, typename ReturnType = void, typename... Args> __forceinline ReturnType CallVT(void* instance, Args... args) {
			using Fn = ReturnType(__thiscall*)(void*, Args...);

			auto function = (*reinterpret_cast<Fn**>(instance))[Index];
			return function(instance, args...);
		}
		template <std::size_t Index, typename ReturnType = void, typename... Args> __forceinline ReturnType CallFromVT(void* instance, void* vtable, Args... args) {
			using Fn = ReturnType(__thiscall*)(void*, Args...);

			auto function = reinterpret_cast<Fn*>(vtable)[Index];
			return function(instance, args...);
		}

		template <typename ReturnType, typename R = ReturnType, typename... Args>
		static ReturnType _SafeCallFunction(const char* moduleName, const char* functionName, R ret, Args... args) {
			using FunctionType = ReturnType(__stdcall*)(Args...);
			FunctionType function = reinterpret_cast<FunctionType>(Utils::Memory::GetProcAddr(moduleName, functionName));
			if (!function)
				return ret;

			return Utils::Memory::SafeExecution::Execute<ReturnType>(reinterpret_cast<uint64_t>(function), ReturnType(), args...);
		}
		template <typename... Args>
		static void _SafeCallFunctionVoid(const char* moduleName, const char* functionName, Args... args) {
			using FunctionType = void(__stdcall*)(Args...);
			FunctionType function = reinterpret_cast<FunctionType>(Utils::Memory::GetProcAddr(moduleName, functionName));
			if (!function)
				return;

			return Utils::Memory::SafeExecution::ExecuteVoid(reinterpret_cast<uint64_t>(function), args...);
		}

		template <typename ReturnType, typename GetOffsetFunc, typename R = ReturnType, typename... Args>
		static ReturnType _SafeCallFunctionOffset(GetOffsetFunc getOffset, R ret, Args... args) {
			using FunctionType = ReturnType(__stdcall*)(Args...);
			FunctionType function = reinterpret_cast<FunctionType>(getOffset());
			if (!function)
				return ret;

			return Utils::Memory::SafeExecution::Execute<ReturnType>(reinterpret_cast<uint64_t>(function), ReturnType(), args...);
		}
		template <typename GetOffsetFunc, typename... Args>
		static void _SafeCallFunctionOffsetVoid(GetOffsetFunc getOffset, Args... args) {
			using FunctionType = void(__stdcall*)(Args...);
			FunctionType function = reinterpret_cast<FunctionType>(getOffset());
			if (!function)
				return;

			return Utils::Memory::SafeExecution::ExecuteVoid(reinterpret_cast<uint64_t>(function), args...);
		}
#pragma endregion
	}
}
