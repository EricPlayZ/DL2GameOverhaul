#include <pch.h>
#include "..\offsets.h"
#include "GameDI_PH.h"
#include "PlayerDI_PH.h"

namespace GamePH {
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
		if (!VerQueryValueA(data.data(), "\\", reinterpret_cast<LPVOID*>(&fileInfo), &fileInfoSize) || !fileInfo)
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
	
	static DWORD64 ShowTPPModelFunc2(GameDI_PH* pGameDI_PH) {
		return _SafeCallFunctionOffset<DWORD64>(Offsets::Get_ShowTPPModelFunc2, 0, pGameDI_PH);
	}
	static void ShowTPPModelFunc3(DWORD64 tppFunc2Addr, bool showTPPModel) {
		PlayerDI_PH* pPlayerDI_PH = PlayerDI_PH::Get();
		if (!pPlayerDI_PH)
			return;

		_SafeCallFunctionOffsetVoid(Offsets::Get_ShowTPPModelFunc3, tppFunc2Addr, showTPPModel);
	}
	void ShowTPPModel(bool showTPPModel) {
		GameDI_PH* pGameDI_PH = GameDI_PH::Get();
		if (!pGameDI_PH)
			return;
		DWORD64 tppFunc2Addr = ShowTPPModelFunc2(pGameDI_PH);
		if (!tppFunc2Addr)
			return;

		ShowTPPModelFunc3(tppFunc2Addr, showTPPModel);
	}
	bool ReloadJumps() {
		return _SafeCallFunctionOffset<bool>(Offsets::Get_ReloadJumps, false);
	}
}
