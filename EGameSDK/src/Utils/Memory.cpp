#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <EGSDK\Utils\Sigscan.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Utils {
	namespace Memory {
		int SafeExecution::fail(unsigned int code, struct _EXCEPTION_POINTERS* ep) {
			if (code == EXCEPTION_ACCESS_VIOLATION)
				return EXCEPTION_EXECUTE_HANDLER;
			else
				return EXCEPTION_CONTINUE_SEARCH;
		}
		
		HMODULE GetCallingDLLModule(void* callerAddress) {
			HMODULE callerModule{};
			if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)callerAddress, &callerModule))
				return callerModule;
			return nullptr;
		}
		const MODULEINFO GetModuleInfo(const char* szModule) {
			if (!szModule)
				return MODULEINFO();

			static std::unordered_map<std::string_view, MODULEINFO> moduleInfoCache;
			auto it = moduleInfoCache.find(szModule);
			if (it != moduleInfoCache.end())
				return it->second;

			HMODULE hModule = GetModuleHandle(szModule);
			if (hModule == 0)
				return MODULEINFO();

			MODULEINFO moduleInfo{};
			GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO));
			moduleInfoCache[szModule] = moduleInfo;

			return moduleInfoCache[szModule];
		}
		const FARPROC GetProcAddr(const std::string_view& module, const std::string_view& funcName) {
			const HMODULE moduleHandle = GetModuleHandleA(module.data());
			if (!moduleHandle)
				return nullptr;

			return GetProcAddress(moduleHandle, funcName.data());
		}

		std::string BytesToIDAPattern(BYTE* bytes, size_t size) {
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
					i += (pattern[static_cast<DWORD64>(i) + 1] == '?') ? 2 : 1;
				} else {
					patt.push_back(pattern[i]);
					i++;
				}
			}

			return patt;
		}
		DWORD64 CalcTargetAddrOfRelativeInstr(DWORD64 addrOfInst, size_t opSize) {
			int offset = *reinterpret_cast<int*>(addrOfInst + opSize);

			return addrOfInst + opSize + 4 + offset;
		}
		std::vector<DWORD64> GetXrefsTo(DWORD64 address, DWORD64 start, size_t size) {
			std::vector<DWORD64> xrefs = {};

			const std::string idaPattern = BytesToIDAPattern(reinterpret_cast<BYTE*>(&address), 8);
			const DWORD64 end = start + size;

			while (start && start < end) {
				DWORD64 xref = reinterpret_cast<DWORD64>(Utils::SigScan::PatternScanner::FindPattern(reinterpret_cast<void*>(start), size, { idaPattern.c_str(), Utils::SigScan::PatternType::Address }));

				if (!xref)
					break;

				xrefs.push_back(xref);
				start = xref + 8;
			}

			return xrefs;
		}
	}
}