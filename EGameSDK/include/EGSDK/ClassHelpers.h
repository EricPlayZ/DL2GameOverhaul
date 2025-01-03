#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <typeinfo>
#include <EGSDK\Utils\Memory.h>
#include <EGSDK\Utils\RTTI.h>
#include <EGSDK\Utils\Values.h>
#include <EGSDK\Exports.h>

namespace EGSDK {
	namespace ClassHelpers {
#pragma pack(1)
		template<size_t size, typename T>
		class buffer {
			char buffer[size];
		public:
			T data;

			operator T() { return data; }
			T operator->() { return data; }

			DWORD64 operator&(const DWORD64 other) const { return reinterpret_cast<DWORD64>(data) & other; }
			DWORD64 operator>>(const int shift) const { return reinterpret_cast<DWORD64>(data) >> shift; }
			DWORD64 operator<<(const int shift) const { return reinterpret_cast<DWORD64>(data) << shift; }

			T& operator=(const T& other) { data = other; return data; }
			T& operator*=(const T& other) { data *= other; return data; }
			T operator*(const T& other) const { return data * other; }
			T& operator/=(const T& other) { data /= other; return data; }
			T operator/(const T& other) const { return data / other; }
			T& operator+=(const T& other) { data += other; return data; }
			T operator+(const T& other) const { return data + other; }
			T& operator-=(const T& other) { data -= other; return data; }
			T operator-(const T& other) const { return data - other; }
		};
#pragma pack()

		extern EGameSDK_API bool IsVftableScanningDisabled();
		extern EGameSDK_API void SetIsVftableScanningDisabled(bool value);

		template <typename T, typename GetOffsetFunc, typename... Args>
		T* _SafeGetImpl(GetOffsetFunc getOffset, bool isDoublePtr, std::string_view className, Args... args) {
			if (!getOffset(args...))
				return nullptr;

			T* ptr = isDoublePtr ? *reinterpret_cast<T**>(getOffset(args...)) : reinterpret_cast<T*>(getOffset(args...));
			if (Utils::Memory::IsBadReadPtr(ptr))
				return nullptr;

			if (IsVftableScanningDisabled())
				return ptr;
			
			if (!className.empty() && !Utils::RTTI::IsClassVTableNameEqualTo(ptr, className))
				return nullptr;
			else if (className.empty() && !Utils::RTTI::IsClassVTableNameEqualTo(ptr, Utils::Values::GetSimpleTypeName(typeid(T).name())))
				return nullptr;

			return ptr;
		}
		template <typename T, typename GetOffsetFunc, typename... Args>
		T* SafeGetter(GetOffsetFunc getOffset, bool isDoublePtr = true, std::string_view className = {}, Args... args) {
			return Utils::Memory::SafeExecution::Execute<T*>(
				reinterpret_cast<uint64_t>(&_SafeGetImpl<T, GetOffsetFunc, Args...>),
				nullptr,
				reinterpret_cast<void*>(getOffset),
				isDoublePtr,
				className,
				args...
			);
		}
	}
	}