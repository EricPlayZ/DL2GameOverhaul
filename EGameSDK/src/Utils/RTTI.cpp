#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <shared_mutex>
#include <DbgHelp.h>
#include <memscan\memscan.h>
#include <EGSDK\Utils\RTTI.h>
#include <EGSDK\Utils\Sigscan.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Utils {
	namespace RTTI {
		static std::unordered_map<DWORD64, std::string> vtableAddressCache{};
		static std::shared_mutex vtableCacheMutex;

        static __forceinline std::string CacheEmptyResult(DWORD64 vtableStructAddr) {
            std::unique_lock lock(vtableCacheMutex);
            vtableAddressCache.try_emplace(vtableStructAddr, "");
            return {};
        }

        std::string GetVTableNameFromVTPtr(void* vtPtr) {
            if (Utils::Memory::IsBadReadPtr(vtPtr))
                return "bad_read_vtable";

            DWORD64 vtableStructAddr = reinterpret_cast<DWORD64>(vtPtr);

            {
                std::shared_lock lock(vtableCacheMutex);
                if (auto it = vtableAddressCache.find(vtableStructAddr); it != vtableAddressCache.end())
                    return it->second;
            }

            DWORD64* objectLocatorPtr = reinterpret_cast<DWORD64*>(vtableStructAddr - sizeof(DWORD64));
            if (Utils::Memory::IsBadReadPtr(objectLocatorPtr))
                return CacheEmptyResult(vtableStructAddr);

            DWORD64 objectLocatorAddr = *objectLocatorPtr;
            DWORD64* baseOffsetPtr = reinterpret_cast<DWORD64*>(objectLocatorAddr + 0x14);
            if (Utils::Memory::IsBadReadPtr(baseOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            DWORD64 baseOffset = *baseOffsetPtr;
            DWORD64 baseAddr = objectLocatorAddr - baseOffset;

            DWORD* classHierarchyDescriptorOffsetPtr = reinterpret_cast<DWORD*>(objectLocatorAddr + 0x10);
            if (Utils::Memory::IsBadReadPtr(classHierarchyDescriptorOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            DWORD classHierarchyDescriptorOffset = *classHierarchyDescriptorOffsetPtr;
            DWORD64 classHierarchyDescriptorAddr = baseAddr + classHierarchyDescriptorOffset;

            int* baseClassCountPtr = reinterpret_cast<int*>(classHierarchyDescriptorAddr + 0x8);
            if (Utils::Memory::IsBadReadPtr(baseClassCountPtr) || *baseClassCountPtr == 0 || *baseClassCountPtr > 24)
                return CacheEmptyResult(vtableStructAddr);

            DWORD* baseClassArrayOffsetPtr = reinterpret_cast<DWORD*>(classHierarchyDescriptorAddr + 0xC);
            if (Utils::Memory::IsBadReadPtr(baseClassArrayOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            DWORD baseClassArrayOffset = *baseClassArrayOffsetPtr;
            DWORD64 baseClassArrayAddr = baseAddr + baseClassArrayOffset;

            DWORD* baseClassDescriptorOffsetPtr = reinterpret_cast<DWORD*>(baseClassArrayAddr);
            if (Utils::Memory::IsBadReadPtr(baseClassDescriptorOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            DWORD baseClassDescriptorOffset = *baseClassDescriptorOffsetPtr;
            DWORD64 baseClassDescriptorAddr = baseAddr + baseClassDescriptorOffset;

            DWORD* typeDescriptorOffsetPtr = reinterpret_cast<DWORD*>(baseClassDescriptorAddr);
            if (Utils::Memory::IsBadReadPtr(typeDescriptorOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            DWORD typeDescriptorOffset = *typeDescriptorOffsetPtr;
            DWORD64 typeDescriptorAddr = baseAddr + typeDescriptorOffset;

            const char* decoratedClassNameCStr = reinterpret_cast<const char*>(typeDescriptorAddr + 0x14);
            std::string decoratedClassName = "?" + std::string(decoratedClassNameCStr);

            char outUndecoratedClassNameCStr[255]{};
            if (UnDecorateSymbolName(decoratedClassName.c_str(), outUndecoratedClassNameCStr, sizeof(outUndecoratedClassNameCStr), UNDNAME_NAME_ONLY) == 0)
                return CacheEmptyResult(vtableStructAddr);

            std::string result(outUndecoratedClassNameCStr);

            // Cache the valid result
            {
                std::unique_lock lock(vtableCacheMutex);
                vtableAddressCache[vtableStructAddr] = result;
            }

            return result;
        }
        std::string GetVTableName(void* classPtr) {
            if (Utils::Memory::IsBadReadPtr(classPtr))
                return "bad_read_class";

            void* vtablePtr = *reinterpret_cast<void**>(classPtr);
            return GetVTableNameFromVTPtr(vtablePtr);
        }
	}
}