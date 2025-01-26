#include <Windows.h>
#include <semaphore>
#include <EGSDK\Utils\Hook.h>
#include <EGSDK\Utils\Memory.h>
#include <EGSDK\Utils\WinMemory.h>

namespace EGSDK::Utils {
	namespace Hook {
#pragma region VTHook
		static int VTHookUnprotect(const void* region) {
			MEMORY_BASIC_INFORMATION mbi{};
			VirtualQuery(region, &mbi, sizeof(mbi));
			VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect);
			return mbi.Protect;
		}
		static void VTHookProtect(const void* region, int protection) {
			MEMORY_BASIC_INFORMATION mbi{};
			VirtualQuery(region, &mbi, sizeof(mbi));
			VirtualProtect(mbi.BaseAddress, mbi.RegionSize, protection, &mbi.Protect);
		}

		void HookVT(void* instance, void* pDetour, void** ppOriginal, uint32_t offset) {
			uint64_t** entry = reinterpret_cast<uint64_t**>(*reinterpret_cast<uint64_t*>(instance) + offset);
			*ppOriginal = *entry;

			const int original_protection = VTHookUnprotect(entry);
			*entry = reinterpret_cast<uint64_t*>(pDetour);
			VTHookProtect(entry, original_protection);
		}
#pragma endregion

#pragma region HookBase
		static std::counting_semaphore<4> maxHookThreads(4);

		HookBase::HookBase(const std::string_view& name, bool hookOnStartup) {
			this->name = new std::string_view(name);
			this->hookOnStartup = hookOnStartup;
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
		std::string_view HookBase::GetName() {
			return *name;
		};

		bool HookBase::IsHooking() {
			return _InterlockedCompareExchange(&isHooking, 0, 0) != 0;
		}
		void HookBase::SetHooking(bool value) {
			_InterlockedExchange(&isHooking, value ? 1 : 0);
		}

		bool HookBase::IsHooked() {
			return _InterlockedCompareExchange(&isHooked, 0, 0) != 0;
		}
		void HookBase::SetHooked(bool value) {
			_InterlockedExchange(&isHooked, value ? 1 : 0);
		}

		bool HookBase::CanHookOnStartup() const {
			return hookOnStartup;
		}
#pragma endregion
	}
}