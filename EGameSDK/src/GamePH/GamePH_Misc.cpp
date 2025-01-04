#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\GamePH_Misc.h>
#include <EGSDK\GamePH\GameDI_PH.h>
#include <EGSDK\GamePH\PlayerDI_PH.h>

namespace EGSDK::GamePH {
	const DWORD GetCurrentGameVersion() {
		char exePath[MAX_PATH]{};
		GetModuleFileNameA(GetModuleHandleA(nullptr), exePath, sizeof(exePath));

		DWORD dummy{};
		const DWORD size = GetFileVersionInfoSizeA(exePath, &dummy);
		if (!size)
			return 0;

		std::vector<BYTE> data(size);
		if (!GetFileVersionInfoA(exePath, 0, size, data.data()))
			return 0;

		VS_FIXEDFILEINFO* fileInfo = nullptr;
		UINT fileInfoSize = 0;
		if (!VerQueryValueA(data.data(), "\\", reinterpret_cast<void**>(&fileInfo), &fileInfoSize) || !fileInfo)
			return 0;

		const DWORD major = HIWORD(fileInfo->dwFileVersionMS);
		const DWORD minor = LOWORD(fileInfo->dwFileVersionMS);
		const DWORD patch = HIWORD(fileInfo->dwFileVersionLS);

		return major * 10000 + minor * 100 + patch;
	}
	const std::string GameVerToStr(DWORD version) {
		DWORD major = version / 10000;
		DWORD minor = (version / 100) % 100;
		DWORD patch = version % 100;

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

		Utils::Memory::SafeCallFunctionOffsetVoid(Offsets::Get_ShowTPPModelFunc3, pPlayerDI_PH, showTPPModel);
	}
	bool ReloadJumps() {
		return Utils::Memory::SafeCallFunctionOffset<bool>(Offsets::Get_ReloadJumps, false);
	}
}
