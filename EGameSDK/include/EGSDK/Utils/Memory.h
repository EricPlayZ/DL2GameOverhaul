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

		class EGameSDK_API SafeExecution {
		public:
			static int fail(unsigned int code, struct _EXCEPTION_POINTERS* ep);

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

		extern EGameSDK_API HMODULE GetCallingDLLModule(void* callerAddress);
		extern EGameSDK_API const MODULEINFO GetModuleInfo(const char* szModule);
		extern EGameSDK_API const FARPROC GetProcAddr(const std::string_view& module, const std::string_view& funcName);

		extern EGameSDK_API std::string BytesToIDAPattern(BYTE* bytes, size_t size);
		extern EGameSDK_API std::string ConvertSigToScannerSig(const char* pattern, int* offsetToAddrInSig = nullptr);

		extern EGameSDK_API DWORD64 CalcTargetAddrOfRelativeInstr(DWORD64 addrOfInst, size_t opSize);
		extern EGameSDK_API std::vector<DWORD64> GetXrefsTo(DWORD64 address, DWORD64 start, size_t size);

#pragma region Templates
		/*
		Copyright (c) 2017 Artem Boldarev <artem.boldarev@gmail.com>

		Permission is hereby granted, free of charge, to any person obtaining a
		copy of this software and associated documentation files(the "Software"),
		to deal in the Software without restriction, including without limitation
		the rights to use, copy, modify, merge, publish, distribute, sublicense,
		and/or sell copies of the Software, and to permit persons to whom the
		Software is furnished to do so, subject to the following conditions :

		The above copyright notice and this permission notice shall be included in
		all copies or substantial portions of the Software.

		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL
		THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
		FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
		DEALINGS IN THE SOFTWARE.
		*/
		// COPYRIGHT NOTICE - START
		template <typename T>
		bool IsBadMemPtr(const bool write, T* ptr, const std::size_t size) {
			const auto min_ptr = 0x10000;
			const auto max_ptr = 0x000F000000000000;

			if (ptr == nullptr || reinterpret_cast<DWORD64>(ptr) < min_ptr || reinterpret_cast<DWORD64>(ptr) >= max_ptr)
				return true;

			DWORD mask = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

			if (write)
				mask = PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

			auto current = reinterpret_cast<BYTE*>(ptr);
			const auto last = current + size;

			// So we are considering the region:
			// [ptr, ptr+size)

			while (current < last) {
				MEMORY_BASIC_INFORMATION mbi;

				// We couldn't get any information on this region.
				// Let's not risk any read/write operation.
				if (VirtualQuery(reinterpret_cast<LPCVOID>(current), &mbi, sizeof mbi) == 0)
					return true;

				// We can't perform our desired read/write operations in this region.
				if ((mbi.Protect & mask) == 0)
					return true;

				// We can't access this region.
				if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
					return true;

				// Let's consider the next region.
				current = reinterpret_cast<BYTE*>(mbi.BaseAddress) + mbi.RegionSize;
			}

			return false;
		}
		template <typename T>
		bool IsBadReadPtr(T* ptr, const std::size_t size) {
			return IsBadMemPtr(false, ptr, size);
		}
		template <typename T>
		bool IsBadReadPtr(T* ptr) {
			return IsBadReadPtr(ptr, sizeof ptr);
		}
		template <typename T>
		bool IsBadWritePtr(T* ptr, const std::size_t size) {
			return IsBadMemPtr(true, ptr, size);
		}
		template <typename T>
		bool IsBadWritePtr(T* ptr) {
			return IsBadWritePtr(ptr, sizeof ptr);
		}
		// COPYRIGHT NOTICE - END

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

		template <typename ReturnType, typename... Args>
		static ReturnType _SafeCallFunction(const char* moduleName, const char* functionName, ReturnType ret, Args... args) {
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

		template <typename ReturnType, typename GetOffsetFunc, typename... Args>
		static ReturnType _SafeCallFunctionOffset(GetOffsetFunc getOffset, ReturnType ret, Args... args) {
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
