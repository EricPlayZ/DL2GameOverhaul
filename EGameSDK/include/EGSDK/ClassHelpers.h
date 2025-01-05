#pragma once
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
        __forceinline T* _SafeGetImpl(GetOffsetFunc getOffset, bool isDoublePtr, bool checkForVTable, Args... args) {
            auto offset = getOffset(args...);
            if (!offset)
                return nullptr;

            T* ptr = isDoublePtr ? *reinterpret_cast<T**>(offset) : reinterpret_cast<T*>(offset);
            if (Utils::Memory::IsBadReadPtr(ptr))
                return nullptr;

            if (!checkForVTable || IsVftableScanningDisabled())
                return ptr;

            static const std::string expectedVtableName = Utils::Values::GetSimpleTypeName(typeid(T).name());
            if (Utils::RTTI::GetVTableNameFromVTPtr(*reinterpret_cast<T**>(ptr)) != expectedVtableName)
                return nullptr;

            return ptr;
        }

        template <typename T, typename GetOffsetFunc, typename... Args>
        __forceinline T* SafeGetter(GetOffsetFunc getOffset, bool isDoublePtr = true, bool checkForVTable = true, Args... args) {
            __try {
				return _SafeGetImpl<T>(getOffset, isDoublePtr, checkForVTable, args...);
            } __except (Utils::Memory::SafeExecution::fail(GetExceptionCode(), GetExceptionInformation())) {
                return nullptr;
            }
        }
    }
}