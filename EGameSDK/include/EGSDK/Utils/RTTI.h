#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace RTTI {
		extern EGameSDK_API std::string GetVTableNameFromVTPtr(void* vtPtr);
		extern EGameSDK_API std::string GetVTableName(void* classPtr);
		extern EGameSDK_API bool IsClassVTableNameEqualTo(void* classPtr, std::string_view tableName);
		extern EGameSDK_API bool IsVTableNameEqualTo(void* vtPtr, std::string_view tableName);
	}
}