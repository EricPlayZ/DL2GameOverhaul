#include <Windows.h>
#include <Psapi.h>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <shared_mutex>
#include <EGSDK\Utils\Sigscan.h>
#include <EGSDK\Utils\Memory.h>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace Memory {
		int SafeExecution::fail(unsigned int code, struct _EXCEPTION_POINTERS* ep) {
			if (code == EXCEPTION_ACCESS_VIOLATION)
				return EXCEPTION_EXECUTE_HANDLER;
			else
				return EXCEPTION_CONTINUE_SEARCH;
		}

		std::string BytesToIDAPattern(uint8_t* bytes, size_t size) {
			std::stringstream idaPattern;
			idaPattern << std::hex << std::uppercase << std::setfill('0');

			for (size_t i = 0; i < size; i++) {
				const int currentByte = bytes[i];
				if (currentByte != SigScanWildCard)
					idaPattern << std::setw(2) << currentByte;
				else
					idaPattern << "??";

				if (i != size - 1)
					idaPattern << " ";
			}

			return idaPattern.str();
		}
		std::string ConvertSigToScannerSig(const char* pattern, int* offsetToAddrInSig) {
			size_t len = strlen(pattern);

			std::string patt{};
			int bytesCounted = 0;

			for (size_t i = 0; i < len;) {
				if (pattern[i] == ' ' || i == len - 1)
					bytesCounted++;

				if (pattern[i] == '[') {
					i++;
					if (offsetToAddrInSig)
						*offsetToAddrInSig = static_cast<int>(bytesCounted);
					continue;
				}

				if (pattern[i] == '?') {
					patt += SigScanWildCardStr;
					i += (pattern[i + 1] == '?') ? 2ULL : 1ULL;
				} else {
					patt.push_back(pattern[i]);
					i++;
				}
			}

			return patt;
		}
		uint64_t CalcTargetAddrOfRelativeInstr(uint64_t addrOfInst, size_t opSize) {
			int offset = *reinterpret_cast<int*>(addrOfInst + opSize);

			return addrOfInst + opSize + 4 + offset;
		}
		std::vector<uint64_t> GetXrefsTo(uint64_t address, uint64_t start, size_t size) {
			std::vector<uint64_t> xrefs = {};

			const std::string idaPattern = BytesToIDAPattern(reinterpret_cast<uint8_t*>(&address), 8);
			const uint64_t end = start + size;

			while (start && start < end) {
				uint64_t xref = reinterpret_cast<uint64_t>(Utils::SigScan::PatternScanner::FindPattern(reinterpret_cast<void*>(start), size, { idaPattern.c_str(), Utils::SigScan::PatternType::Address }));

				if (!xref)
					break;

				xrefs.push_back(xref);
				start = xref + 8;
			}

			return xrefs;
		}

		template <typename T>
		bool IsBadMemPtr(bool write, T* ptr, size_t size) {
			struct PointerHash {
				size_t operator()(const void* ptr) const {
					return reinterpret_cast<size_t>(ptr) >> 12; // Reduce collisions by using shifted addresses.
				}
			};

			static std::unordered_map<void*, MEMORY_BASIC_INFORMATION, PointerHash> memoryCache{};
			static std::shared_mutex cacheMutex{};

			constexpr uint64_t min_ptr = 0x10000;
			constexpr uint64_t max_ptr = 0x000F000000000000;

			if (ptr == nullptr || reinterpret_cast<uint64_t>(ptr) < min_ptr || reinterpret_cast<uint64_t>(ptr) >= max_ptr)
				return true;

			uint32_t mask = write
				? (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
				: (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);

			uint8_t* current = reinterpret_cast<uint8_t*>(ptr);
			const uint8_t* last = current + size;

			while (current < last) {
				MEMORY_BASIC_INFORMATION mbi{};
				{
					std::shared_lock lock(cacheMutex);
					auto pageAlignedAddr = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(current) & ~0xFFF);
					auto it = memoryCache.find(pageAlignedAddr);
					if (it != memoryCache.end())
						mbi = it->second;
					else {
						lock.unlock();
						if (VirtualQuery(reinterpret_cast<const void*>(pageAlignedAddr), &mbi, sizeof mbi) == 0)
							return true;

						std::unique_lock writeLock(cacheMutex);
						memoryCache[pageAlignedAddr] = mbi;
					}
				}

				if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS) || (mbi.Protect & mask) == 0)
					return true;

				current = reinterpret_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
			}

			return false;
		}

		template EGameSDK_API bool IsBadMemPtr<void>(bool write, void* ptr, size_t size);
	}
}