#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace RTTI {
		extern EGameSDK_API DWORD64 GetVTablePtr(const char* moduleName, const char* tableName);
	}
}