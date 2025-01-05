#include <Windows.h>
#include <semaphore>
#include <EGSDK\Utils\Hook.h>
#include <EGSDK\Utils\Memory.h>

namespace EGSDK::Utils {
	namespace Hook {
#pragma region VTHook
		static int VTHookUnprotect(LPCVOID region) {
			MEMORY_BASIC_INFORMATION mbi{};
			VirtualQuery(region, &mbi, sizeof(mbi));
			VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect);
			return mbi.Protect;
		}
		static void VTHookProtect(LPCVOID region, int protection) {
			MEMORY_BASIC_INFORMATION mbi{};
			VirtualQuery(region, &mbi, sizeof(mbi));
			VirtualProtect(mbi.BaseAddress, mbi.RegionSize, protection, &mbi.Protect);
		}

		void HookVT(void* instance, void* pDetour, void** ppOriginal, const DWORD offset) {
			PDWORD64* entry = reinterpret_cast<PDWORD64*>(*reinterpret_cast<DWORD64*>(instance) + offset);
			*ppOriginal = *entry;

			const int original_protection = VTHookUnprotect(entry);
			*entry = reinterpret_cast<PDWORD64>(pDetour);
			VTHookProtect(entry, original_protection);
		}
#pragma endregion

#pragma region BPHook
		static std::vector<BreakpointHook*> s_hookList;

		BreakpointHook::BreakpointHook(PDWORD64 addr, void (*handler)(PEXCEPTION_POINTERS)) {
			m_addr = addr;
			m_handler = handler;
			m_originalBytes = *reinterpret_cast<BYTE*>(m_addr);

			AddVectoredExceptionHandler(true, (PVECTORED_EXCEPTION_HANDLER)OnException);

			s_hookList.push_back(this);

			Enable();
		}
		void BreakpointHook::Enable() {
			DWORD oldProtection = 0;

			VirtualProtect(m_addr, 1, PAGE_EXECUTE_READWRITE, &m_originalProtection);
			*reinterpret_cast<BYTE*>(m_addr) = 0xCC;
			VirtualProtect(m_addr, 1, m_originalProtection, &oldProtection);
		}
		void BreakpointHook::Disable() {
			DWORD oldProtection = 0;

			VirtualProtect(m_addr, 1, PAGE_EXECUTE_READWRITE, &oldProtection);
			*reinterpret_cast<BYTE*>(m_addr) = m_originalBytes;
			VirtualProtect(m_addr, 1, m_originalProtection, &oldProtection);
		}
		BreakpointHook::~BreakpointHook() {
			Disable();

			auto it = std::find(s_hookList.begin(), s_hookList.end(), this);

			if (it != s_hookList.end())
				s_hookList.erase(it);
		}

		static PDWORD64 lastBpAddress = nullptr;
		long WINAPI BreakpointHook::OnException(PEXCEPTION_POINTERS info) {
			for (auto it = s_hookList.begin(); it != s_hookList.end(); it++) {
				BreakpointHook* bp = *it;

				if (bp->m_addr == info->ExceptionRecord->ExceptionAddress) {
					bp->Disable();

					lastBpAddress = bp->m_addr;
					info->ContextRecord->EFlags |= 0x100; // Set EFLAGS to single step

					return EXCEPTION_CONTINUE_EXECUTION;
				} else if (info->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP && bp->m_addr == lastBpAddress) {
					bp->Enable();
					info->ContextRecord->EFlags &= ~0x00000100; // Remove TRACE from EFLAGS

					bp->m_handler(info);

					return EXCEPTION_CONTINUE_EXECUTION;
				}
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}
#pragma endregion

#pragma region HookBase
		static std::counting_semaphore<4> maxHookThreads(4);

		HookBase::HookBase(const std::string_view& name) {
			this->name = new std::string_view(name);
			HMODULE hModule = Memory::GetCallingDLLModule(this);
			(*GetInstances())[hModule].insert(this);
		}
		HookBase::~HookBase() {
			HMODULE hModule = Memory::GetCallingDLLModule(this);
			(*GetInstances())[hModule].erase(this);
			delete name;
		}
		std::unordered_map<HMODULE, std::set<HookBase*>>* HookBase::GetInstances() {
			static std::unordered_map<HMODULE, std::set<HookBase*>> instances{};
			return &instances;
		};

		bool HookBase::IsHooking() { return _InterlockedCompareExchange(&isHooking, 0, 0) != 0; }
		void HookBase::SetHooking(bool value) { _InterlockedExchange(&isHooking, value ? 1 : 0); }

		bool HookBase::IsHooked() { return _InterlockedCompareExchange(&isHooked, 0, 0) != 0; }
		void HookBase::SetHooked(bool value) { _InterlockedExchange(&isHooked, value ? 1 : 0); }
#pragma endregion
	}
}