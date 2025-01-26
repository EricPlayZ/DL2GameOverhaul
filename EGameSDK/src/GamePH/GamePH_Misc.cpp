#include <Windows.h>
#include <string>
#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\GamePH_Misc.h>
#include <EGSDK\GamePH\GameDI_PH.h>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\Utils\WinMemory.h>

namespace EGSDK::GamePH {
	const uint32_t GetCurrentGameVersion() {
		char exePath[MAX_PATH]{};
		GetModuleFileName(GetModuleHandle(nullptr), exePath, sizeof(exePath));

		unsigned long dummy{};
		const uint32_t size = GetFileVersionInfoSize(exePath, &dummy);
		if (!size)
			return 0;

		std::vector<uint8_t> data(size);
		if (!GetFileVersionInfo(exePath, 0, size, data.data()))
			return 0;

		VS_FIXEDFILEINFO* fileInfo = nullptr;
		UINT fileInfoSize = 0;
		if (!VerQueryValue(data.data(), "\\", reinterpret_cast<void**>(&fileInfo), &fileInfoSize) || !fileInfo)
			return 0;

		const uint32_t major = HIWORD(fileInfo->dwFileVersionMS);
		const uint32_t minor = LOWORD(fileInfo->dwFileVersionMS);
		const uint32_t patch = HIWORD(fileInfo->dwFileVersionLS);

		return major * 10000 + minor * 100 + patch;
	}
	const std::string GameVerToStr(uint32_t version) {
		uint32_t major = version / 10000;
		uint32_t minor = (version / 100) % 100;
		uint32_t patch = version % 100;

		return std::string(std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch));
	}
	const std::string GetCurrentGameVersionStr() {
		if (!GetCurrentGameVersion())
			return "UNKNOWN";

		return GameVerToStr(GetCurrentGameVersion());
	}

	void ShowTPPModel(bool showTPPModel) {
		GameDI_PH* pGameDI_PH = GameDI_PH::Get();
		if (!pGameDI_PH)
			return;
		PlayerDI_PH* pPlayerDI_PH = PlayerDI_PH::Get();
		if (!pPlayerDI_PH)
			return;

		Utils::Memory::SafeCallFunctionOffsetVoid(OffsetManager::Get_ShowTPPModelFunc3, pPlayerDI_PH, showTPPModel);
	}
	bool ReloadJumps() {
		return Utils::Memory::SafeCallFunctionOffset<bool>(OffsetManager::Get_ReloadJumps, false);
	}
}
