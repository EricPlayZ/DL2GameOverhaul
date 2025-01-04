#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace RTTI {
		extern EGameSDK_API std::string GetVTableNameFromVTPtr(void* vtPtr);
		extern EGameSDK_API std::string GetVTableName(void* classPtr);
	}
}