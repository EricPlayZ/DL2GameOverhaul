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

#define SAFE_DEPENDENT_OFFSET_GETTER(ClassName, DependentClass) \
    static ClassName* GetOffset_##ClassName() { \
		DependentClass* dependentInstance = DependentClass::Get(); \
		return dependentInstance ? dependentInstance->p##ClassName : nullptr; \
    }

#define SafeGetter(ClassName, ModuleName) \
	ClassName* ClassName::Get() { \
		return _SafeGetter<ClassName>(Offsets::Get_##ClassName, ModuleName, true, nullptr); \
	}
#define SafeGetterVT(ClassName, ModuleName) \
	ClassName* ClassName::Get() { \
		return _SafeGetter<ClassName>(Offsets::Get_##ClassName, ModuleName, true, Offsets::GetVT_##ClassName); \
	}

#define SafeGetterDep(ClassName, DependentClass, ModuleName) \
	SAFE_DEPENDENT_OFFSET_GETTER(ClassName, DependentClass) \
	ClassName* ClassName::Get() { \
		return _SafeGetter<ClassName>(GetOffset_##ClassName, ModuleName, false, nullptr); \
	}
#define SafeGetterDepVT(ClassName, DependentClass, ModuleName) \
	SAFE_DEPENDENT_OFFSET_GETTER(ClassName, DependentClass) \
	ClassName* ClassName::Get() { \
		return _SafeGetter<ClassName>(GetOffset_##ClassName, ModuleName, false, Offsets::GetVT_##ClassName); \
	}

#define SafeGetterDepCustom(ClassName, OffsetGetter, IsDoublePtr, ModuleName) \
	ClassName* ClassName::Get() { \
		return _SafeGetter<ClassName>(OffsetGetter, ModuleName, IsDoublePtr, nullptr); \
	}
#define SafeGetterDepCustomVT(ClassName, OffsetGetter, IsDoublePtr, ModuleName) \
	ClassName* ClassName::Get() { \
		return _SafeGetter<ClassName>(OffsetGetter, ModuleName, IsDoublePtr, Offsets::GetVT_##ClassName); \
	}