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
		static std::unordered_map<uint64_t, std::string> vtableAddressCache{};
		static std::shared_mutex vtableCacheMutex;

        static __forceinline std::string CacheEmptyResult(uint64_t vtableStructAddr) {
            std::unique_lock lock(vtableCacheMutex);
            vtableAddressCache.try_emplace(vtableStructAddr, "");
            return {};
        }

        std::string GetVTableNameFromVTPtr(void* vtPtr) {
            if (Utils::Memory::IsBadReadPtr(vtPtr))
                return "bad_read_vtable";

            uint64_t vtableStructAddr = reinterpret_cast<uint64_t>(vtPtr);

            {
                std::shared_lock lock(vtableCacheMutex);
                if (auto it = vtableAddressCache.find(vtableStructAddr); it != vtableAddressCache.end())
                    return it->second;
            }

            uint64_t* objectLocatorPtr = reinterpret_cast<uint64_t*>(vtableStructAddr - sizeof(uint64_t));
            if (Utils::Memory::IsBadReadPtr(objectLocatorPtr))
                return CacheEmptyResult(vtableStructAddr);

            uint64_t objectLocatorAddr = *objectLocatorPtr;
            uint64_t* baseOffsetPtr = reinterpret_cast<uint64_t*>(objectLocatorAddr + 0x14);
            if (Utils::Memory::IsBadReadPtr(baseOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            uint64_t baseOffset = *baseOffsetPtr;
            uint64_t baseAddr = objectLocatorAddr - baseOffset;

            uint32_t* classHierarchyDescriptorOffsetPtr = reinterpret_cast<uint32_t*>(objectLocatorAddr + 0x10);
            if (Utils::Memory::IsBadReadPtr(classHierarchyDescriptorOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            uint32_t classHierarchyDescriptorOffset = *classHierarchyDescriptorOffsetPtr;
            uint64_t classHierarchyDescriptorAddr = baseAddr + classHierarchyDescriptorOffset;

            int* baseClassCountPtr = reinterpret_cast<int*>(classHierarchyDescriptorAddr + 0x8);
            if (Utils::Memory::IsBadReadPtr(baseClassCountPtr) || *baseClassCountPtr == 0 || *baseClassCountPtr > 24)
                return CacheEmptyResult(vtableStructAddr);

            uint32_t* baseClassArrayOffsetPtr = reinterpret_cast<uint32_t*>(classHierarchyDescriptorAddr + 0xC);
            if (Utils::Memory::IsBadReadPtr(baseClassArrayOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            uint32_t baseClassArrayOffset = *baseClassArrayOffsetPtr;
            uint64_t baseClassArrayAddr = baseAddr + baseClassArrayOffset;

            uint32_t* baseClassDescriptorOffsetPtr = reinterpret_cast<uint32_t*>(baseClassArrayAddr);
            if (Utils::Memory::IsBadReadPtr(baseClassDescriptorOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            uint32_t baseClassDescriptorOffset = *baseClassDescriptorOffsetPtr;
            uint64_t baseClassDescriptorAddr = baseAddr + baseClassDescriptorOffset;

            uint32_t* typeDescriptorOffsetPtr = reinterpret_cast<uint32_t*>(baseClassDescriptorAddr);
            if (Utils::Memory::IsBadReadPtr(typeDescriptorOffsetPtr))
                return CacheEmptyResult(vtableStructAddr);

            uint32_t typeDescriptorOffset = *typeDescriptorOffsetPtr;
            uint64_t typeDescriptorAddr = baseAddr + typeDescriptorOffset;

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