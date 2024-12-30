#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK {
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

	template <typename T, typename GetOffsetFunc, typename GetVTFunc = std::nullptr_t, typename... Args>
	static T* _SafeGetImpl(GetOffsetFunc getOffset, const char* moduleName, bool isDoublePtr, GetVTFunc getVT, Args... args) {
		if (!getOffset(args...))
			return nullptr;

		T* ptr = isDoublePtr ? *reinterpret_cast<T**>(getOffset(args...)) : reinterpret_cast<T*>(getOffset(args...));
		if (moduleName && !Utils::Memory::IsValidPtrMod(ptr, moduleName))
			return nullptr;

		if constexpr (!std::is_same_v<GetVTFunc, std::nullptr_t>) {
			if (*reinterpret_cast<DWORD64**>(ptr) != getVT())
				return nullptr;
		}

		return ptr;
	}
	template <typename T, typename GetOffsetFunc, typename GetVTFunc = std::nullptr_t, typename... Args>
	static T* _SafeGetter(GetOffsetFunc getOffset, const char* moduleName, bool isDoublePtr = true, GetVTFunc getVT = nullptr, Args... args) {
		return Utils::Memory::SafeExecution::Execute<T*>(
			reinterpret_cast<uint64_t>(&_SafeGetImpl<T, GetOffsetFunc, GetVTFunc, Args...>),
			nullptr,
			reinterpret_cast<void*>(getOffset),
			moduleName,
			isDoublePtr,
			reinterpret_cast<void*>(getVT),
			args...
		);
	}
}