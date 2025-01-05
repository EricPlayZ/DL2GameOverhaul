#include <Windows.h>
#include <Psapi.h>
#include <list>
#include <memscan\memscan.h>
#include <EGSDK\Utils\Sigscan.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Utils {
	namespace SigScan {
		void* PatternScanner::FindPattern(const std::string_view moduleName, const Pattern& pattern) {
			HMODULE hModule = GetModuleHandleA(moduleName.data());
			if (!hModule)
				return nullptr;

			return PatternScanner::FindPattern(hModule, Utils::Memory::GetModuleInfo(moduleName.data()).SizeOfImage, pattern);
		}
		void* PatternScanner::FindPattern(const Pattern& pattern) {
			MODULEINFO hModuleInfo;
			GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(nullptr), &hModuleInfo, sizeof(hModuleInfo));
			return PatternScanner::FindPattern(GetModuleHandleA(nullptr), hModuleInfo.SizeOfImage, pattern);
		}

		std::vector<void*> PatternScanner::FindPatterns(const std::string_view moduleName, const Pattern& pattern) {
			HMODULE hModule = GetModuleHandleA(moduleName.data());
			if (!hModule)
				return {};

			return PatternScanner::FindPatterns(hModule, Utils::Memory::GetModuleInfo(moduleName.data()).SizeOfImage, pattern);
		}
		std::vector<void*> PatternScanner::FindPatterns(void* startAddress, DWORD64 searchSize, const Pattern& pattern) {
			std::vector<void*> ret;

			void* base = startAddress;
			DWORD64 size = searchSize;
			void* addr = base;
			do {
				addr = PatternScanner::FindPattern(reinterpret_cast<void*>((reinterpret_cast<DWORD64>(addr) + 1)), size - (reinterpret_cast<DWORD64>(addr) - reinterpret_cast<DWORD64>(base) + 1), pattern);
				if (addr)
					ret.push_back(addr);
			} while (!addr);

			return ret;
		}

		void* PatternScanner::FindPattern(void* startAddress, DWORD64 searchSize, const Pattern& pattern) {
			int offset = 0;
			std::string patt = Memory::ConvertSigToScannerSig(pattern.pattern, &offset);

			const auto scanner = memscan::mapped_region_t(reinterpret_cast<DWORD64>(startAddress), reinterpret_cast<DWORD64>(startAddress) + searchSize);
			auto patternFind = scanner.find_pattern<ms_uptr_t>(patt);

			void* ret = nullptr;

			if (patternFind.has_value())
				ret = reinterpret_cast<void*>(patternFind.value() + offset);

			switch (pattern.type) {
			case PatternType::Pointer:
				ret = PatternScanner::ResolvePtr<DWORD64>(ret);
				break;

			case PatternType::PointerBYTE:
				ret = PatternScanner::ResolvePtr<BYTE>(ret);
				break;

			case PatternType::PointerWORD:
				ret = PatternScanner::ResolvePtr<WORD>(ret);
				break;

			case PatternType::PointerQWORD:
				ret = PatternScanner::ResolvePtr<DWORD64>(ret);
				break;

			case PatternType::RelativePointer:
				ret = PatternScanner::ResolveRelativePtr<int32_t>(ret);
				break;

			case PatternType::RelativePointerBYTE:
				ret = PatternScanner::ResolveRelativePtr<BYTE>(ret);
				break;

			case PatternType::RelativePointerWORD:
				ret = PatternScanner::ResolveRelativePtr<WORD>(ret);
				break;

			case PatternType::RelativePointerQWORD:
				ret = PatternScanner::ResolveRelativePtr<DWORD64>(ret);
				break;
			default:
				break;
			}

			return ret;
		}

		void* PatternScanner::FindPattern(void* startAddress, DWORD64 searchSize, const Pattern* patterns, float* ratio) {
			int totalCount = 0;
			struct result {
				void* addr;
				int count;
			};
			std::list<result> results;

			int bestCount = 0;
			void* ret = nullptr;

			while (true) {
				Pattern pattern = patterns[totalCount];
				if (!pattern.pattern)
					break;

				totalCount++;

				void* addr = PatternScanner::FindPattern(startAddress, searchSize, pattern);
				if (!addr)
					continue;

				bool found = false;
				auto it = std::find_if(results.begin(), results.end(), [addr](result& res) { return res.addr == addr; });
				if (it != results.end()) {
					it->count++;

					if (it->count > bestCount) {
						ret = addr;
						bestCount = it->count;
					}

					found = true;
				}

				if (!found) {
					result res = { addr, 1 };
					results.push_back(res);

					if (bestCount == 0) {
						bestCount = 1;
						ret = addr;
					}
				}
			}

			if (ratio)
				*ratio = static_cast<float>(bestCount) / totalCount;

			return ret;
		}
		void* PatternScanner::FindPattern(const std::string_view ModuleName, Pattern* patterns, float* ratio) {
			return PatternScanner::FindPattern(GetModuleHandle(ModuleName.data()), Utils::Memory::GetModuleInfo(ModuleName.data()).SizeOfImage, patterns, ratio);
		}
	}
}