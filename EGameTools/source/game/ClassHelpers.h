#pragma once
#include "..\utils\memory.h"
#pragma pack(1)

template<size_t size, typename T> class buffer {
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

template <typename T, typename GetOffsetFunc, typename GetVTFunc = std::nullptr_t>
static T* SafeGetImpl(GetOffsetFunc getOffset, const char* moduleName, GetVTFunc getVT) {
	if (!getOffset())
		return nullptr;

	T* ptr = *reinterpret_cast<T**>(getOffset());
	if (!Utils::Memory::IsValidPtrMod(ptr, moduleName))
		return nullptr;

	if constexpr (!std::is_same_v<GetVTFunc, std::nullptr_t>) {
		if (*reinterpret_cast<DWORD64**>(ptr) != getVT())
			return nullptr;
	}

	return ptr;
}

template <typename T, typename GetOffsetFunc, typename GetVTFunc>
static T* SafeGetImplWrapper(void* getOffset, const char* moduleName, void* getVT) {
	auto actualGetOffset = reinterpret_cast<GetOffsetFunc>(getOffset);
	GetVTFunc actualGetVT = nullptr;

	if constexpr (!std::is_same_v<GetVTFunc, std::nullptr_t>) {
		actualGetVT = reinterpret_cast<GetVTFunc>(getVT);
	}

	return SafeGetImpl<T, GetOffsetFunc, GetVTFunc>(actualGetOffset, moduleName, actualGetVT);
}

template <typename T, typename GetOffsetFunc, typename GetVTFunc = std::nullptr_t>
T* SafeGetter(GetOffsetFunc getOffset, const char* moduleName, GetVTFunc getVT = nullptr) {
	return Utils::Memory::SafeExecution::Execute<T*>(
		reinterpret_cast<uint64_t>(&SafeGetImplWrapper<T, GetOffsetFunc, GetVTFunc>),
		nullptr,
		reinterpret_cast<void*>(getOffset),
		moduleName,
		reinterpret_cast<void*>(getVT)
	);
}