#pragma once
#include <excpt.h>
#include <string>
#include <string_view>
#include <vector>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace Memory {
		static constexpr uint8_t SigScanWildCard = 0xAA;
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

		extern EGameSDK_API std::string BytesToIDAPattern(uint8_t* bytes, size_t size);
		extern EGameSDK_API std::string ConvertSigToScannerSig(const char* pattern, int* offsetToAddrInSig = nullptr);

		extern EGameSDK_API uint64_t CalcTargetAddrOfRelativeInstr(uint64_t addrOfInst, size_t opSize);
		extern EGameSDK_API std::vector<uint64_t> GetXrefsTo(uint64_t address, uint64_t start, size_t size);

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
		extern bool IsBadMemPtr(bool write, T* ptr, size_t size);
		__forceinline bool IsBadReadPtr(void* ptr, const size_t size) {
			return IsBadMemPtr(false, ptr, size);
		}
		__forceinline bool IsBadReadPtr(void* ptr) {
			return IsBadReadPtr(ptr, sizeof(ptr));
		}
		__forceinline bool IsBadWritePtr(void* ptr, const size_t size) {
			return IsBadMemPtr(true, ptr, size);
		}
		__forceinline bool IsBadWritePtr(void* ptr) {
			return IsBadWritePtr(ptr, sizeof(ptr));
		}
		// COPYRIGHT NOTICE - END

		template <size_t Index, typename ReturnType = void, typename... Args>
		__forceinline ReturnType CallVT(void* instance, Args... args) {
			using Fn = ReturnType(__thiscall*)(void*, Args...);

			auto function = (*reinterpret_cast<Fn**>(instance))[Index];
			return function(instance, args...);
		}
		template <size_t Index, typename ReturnType = void, typename... Args>
		__forceinline ReturnType CallFromVT(void* instance, void* vtable, Args... args) {
			using Fn = ReturnType(__thiscall*)(void*, Args...);

			auto function = reinterpret_cast<Fn*>(vtable)[Index];
			return function(instance, args...);
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
