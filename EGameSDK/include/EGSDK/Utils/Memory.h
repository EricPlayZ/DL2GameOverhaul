#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace Memory {
		static constexpr BYTE SigScanWildCard = 0xAA;
		static constexpr std::string_view SigScanWildCardStr = "AA";

		class EGameSDK_API SafeExecution {
		public:
			static int fail(unsigned int code, struct _EXCEPTION_POINTERS* ep);

			template<typename T = void*, typename... Args>
			static T Execute(uint64_t ptr, T ret, Args... args) {
				__try {
					using FunctionType = T(__stdcall*)(Args...);
					auto func = reinterpret_cast<FunctionType>(ptr);
					return func(args...);
				} __except (fail(GetExceptionCode(), GetExceptionInformation())) {
					return ret;
				}
			}

			template<typename... Args>
			static void ExecuteVoid(uint64_t ptr, Args... args) {
				__try {
					using FunctionType = void(__stdcall*)(Args...);
					auto func = reinterpret_cast<FunctionType>(ptr);
					func(args...);
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
		bool IsBadMemPtr(bool write, T* ptr, std::size_t size) {
			struct PointerHash {
				std::size_t operator()(const void* ptr) const {
					return reinterpret_cast<std::size_t>(ptr) >> 12; // Reduce collisions by using shifted addresses.
				}
			};

			static std::unordered_map<void*, MEMORY_BASIC_INFORMATION, PointerHash> memoryCache{};
			static std::shared_mutex cacheMutex{};

			constexpr DWORD64 min_ptr = 0x10000;
			constexpr DWORD64 max_ptr = 0x000F000000000000;

			if (ptr == nullptr || reinterpret_cast<DWORD64>(ptr) < min_ptr || reinterpret_cast<DWORD64>(ptr) >= max_ptr)
				return true;

			DWORD mask = write
				? (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
				: (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);

			BYTE* current = reinterpret_cast<BYTE*>(ptr);
			const BYTE* last = current + size;

			while (current < last) {
				MEMORY_BASIC_INFORMATION mbi{};
				{
					std::shared_lock lock(cacheMutex);
					auto pageAlignedAddr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(current) & ~0xFFF);
					auto it = memoryCache.find(pageAlignedAddr);
					if (it != memoryCache.end())
						mbi = it->second;
					else {
						lock.unlock();
						if (VirtualQuery(reinterpret_cast<LPCVOID>(pageAlignedAddr), &mbi, sizeof mbi) == 0)
							return true;

						std::unique_lock writeLock(cacheMutex);
						memoryCache[pageAlignedAddr] = mbi;
					}
				}

				if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS) || (mbi.Protect & mask) == 0)
					return true;

				current = reinterpret_cast<BYTE*>(mbi.BaseAddress) + mbi.RegionSize;
			}

			return false;
		}
		template <typename T>
		__forceinline bool IsBadReadPtr(T* ptr, const std::size_t size) {
			return IsBadMemPtr(false, ptr, size);
		}
		template <typename T>
		__forceinline bool IsBadReadPtr(T* ptr) {
			return IsBadReadPtr(ptr, sizeof(ptr));
		}
		template <typename T>
		__forceinline bool IsBadWritePtr(T* ptr, const std::size_t size) {
			return IsBadMemPtr(true, ptr, size);
		}
		template <typename T>
		__forceinline bool IsBadWritePtr(T* ptr) {
			return IsBadWritePtr(ptr, sizeof(ptr));
		}
		// COPYRIGHT NOTICE - END

		template <std::size_t Index, typename ReturnType = void, typename... Args>
		__forceinline ReturnType CallVT(void* instance, Args... args) {
			using Fn = ReturnType(__thiscall*)(void*, Args...);

			auto function = (*reinterpret_cast<Fn**>(instance))[Index];
			return function(instance, args...);
		}
		template <std::size_t Index, typename ReturnType = void, typename... Args>
		__forceinline ReturnType CallFromVT(void* instance, void* vtable, Args... args) {
			using Fn = ReturnType(__thiscall*)(void*, Args...);

			auto function = reinterpret_cast<Fn*>(vtable)[Index];
			return function(instance, args...);
		}

		template <typename ReturnType, typename... Args>
		__forceinline ReturnType SafeCallFunction(const char* moduleName, const char* functionName, ReturnType ret, Args... args) {
			using FunctionType = ReturnType(__stdcall*)(Args...);
			FunctionType function = reinterpret_cast<FunctionType>(Utils::Memory::GetProcAddr(moduleName, functionName));
			if (!function)
				return ret;

			__try {
				return function(args...);
			} __except (SafeExecution::fail(GetExceptionCode(), GetExceptionInformation())) {
				return ret;
			}
		}
		template <typename... Args>
		__forceinline void SafeCallFunctionVoid(const char* moduleName, const char* functionName, Args... args) {
			using FunctionType = void(__stdcall*)(Args...);
			FunctionType function = reinterpret_cast<FunctionType>(Utils::Memory::GetProcAddr(moduleName, functionName));
			if (!function)
				return;

			__try {
				function(args...);
			} __except (SafeExecution::fail(GetExceptionCode(), GetExceptionInformation())) {
				return;
			}
		}

		template <typename ReturnType, typename GetOffsetFunc, typename... Args>
		__forceinline ReturnType SafeCallFunctionOffset(GetOffsetFunc getOffset, ReturnType ret, Args... args) {
			using FunctionType = ReturnType(__stdcall*)(Args...);
			FunctionType function = reinterpret_cast<FunctionType>(getOffset());
			if (!function)
				return ret;

			__try {
				return function(args...);
			} __except (SafeExecution::fail(GetExceptionCode(), GetExceptionInformation())) {
				return ret;
			}
		}
		template <typename GetOffsetFunc, typename... Args>
		__forceinline void SafeCallFunctionOffsetVoid(GetOffsetFunc getOffset, Args... args) {
			using FunctionType = void(__stdcall*)(Args...);
			FunctionType function = reinterpret_cast<FunctionType>(getOffset());
			if (!function)
				return;

			__try {
				function(args...);
			} __except (SafeExecution::fail(GetExceptionCode(), GetExceptionInformation())) {
				return;
			}
		}
#pragma endregion
	}
}
