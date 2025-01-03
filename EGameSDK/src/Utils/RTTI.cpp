#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <DbgHelp.h>
#include <memscan\memscan.h>
#include <EGSDK\Utils\RTTI.h>
#include <EGSDK\Utils\Sigscan.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Utils {
	namespace RTTI {
		static std::unordered_map<DWORD64, std::string> vtableAddressCache{};

		static std::string _GetVTableNameFromVTPtr(void* vtPtr) {
			if (Utils::Memory::IsBadReadPtr(vtPtr))
				return "bad_read_vtable";
			DWORD64 vtableStructAddr = reinterpret_cast<DWORD64>(vtPtr);
			if (auto it = vtableAddressCache.find(vtableStructAddr); it != vtableAddressCache.end())
				return it->second;

			DWORD64 objectLocatorAddr = *reinterpret_cast<DWORD64*>(vtableStructAddr - sizeof(DWORD64));

			DWORD64 baseOffset = *reinterpret_cast<DWORD64*>(objectLocatorAddr + 0x14);
			DWORD64 baseAddr = objectLocatorAddr - baseOffset;

			DWORD classHierarchyDescriptorOffset = *reinterpret_cast<DWORD*>(objectLocatorAddr + 0x10);
			DWORD64 classHierarchyDescriptorAddr = baseAddr + classHierarchyDescriptorOffset;

			int baseClassCount = *reinterpret_cast<int*>(classHierarchyDescriptorAddr + 0x8);
			if (baseClassCount == 0 || baseClassCount > 24)
				return {};

			DWORD baseClassArrayOffset = *reinterpret_cast<DWORD*>(classHierarchyDescriptorAddr + 0xC);
			DWORD64 baseClassArrayAddr = baseAddr + baseClassArrayOffset;

			DWORD baseClassDescriptorOffset = *reinterpret_cast<DWORD*>(baseClassArrayAddr);
			DWORD64 baseClassDescriptorAddr = baseAddr + baseClassDescriptorOffset;

			DWORD typeDescriptorOffset = *reinterpret_cast<DWORD*>(baseClassDescriptorAddr);
			DWORD64 typeDescriptorAddr = baseAddr + typeDescriptorOffset;

			std::string decoratedClassName = "?" + std::string(reinterpret_cast<const char*>(typeDescriptorAddr + 0x14));
			char outUndecoratedClassName[255]{};
			UnDecorateSymbolName(decoratedClassName.c_str(), outUndecoratedClassName, sizeof(outUndecoratedClassName), UNDNAME_NAME_ONLY);

			vtableAddressCache[vtableStructAddr] = outUndecoratedClassName;

			return outUndecoratedClassName;
		}
		static std::string _GetVTableName(void* classPtr) {
			if (Utils::Memory::IsBadReadPtr(classPtr))
				return "bad_read_class";
			return _GetVTableNameFromVTPtr(*reinterpret_cast<DWORD64**>(classPtr));
		}

		std::string GetVTableNameFromVTPtr(void* vtPtr) {
			std::string result = _GetVTableNameFromVTPtr(vtPtr);
			if (result.empty())
				vtableAddressCache.try_emplace(*reinterpret_cast<DWORD64*>(vtPtr), "");

			return result;
		}
		std::string GetVTableName(void* classPtr) {
			std::string result = _GetVTableName(classPtr);
			if (result.empty())
				vtableAddressCache.try_emplace(*reinterpret_cast<DWORD64*>(classPtr), "");

			return result;
		}
		bool IsClassVTableNameEqualTo(void* classPtr, std::string_view tableName) { return GetVTableName(classPtr) == tableName; }
		bool IsVTableNameEqualTo(void* vtPtr, std::string_view tableName) { return GetVTableNameFromVTPtr(vtPtr) == tableName; }
	}
}