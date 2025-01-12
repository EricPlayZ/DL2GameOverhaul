#pragma once
#include <string>
#include <vector>
#include <EGSDK\Vector3.h>
#include <EGSDK\Utils\Hook.h>

namespace EGT::Engine {
	namespace Hooks {
		extern bool switchedFreeCamByGamePause;
		extern EGSDK::Vector3 freeCamPosBeforeGamePause;
		extern int mountDataPaksRanWith8Count;

		extern EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, DWORD, DWORD), DWORD64, DWORD, DWORD> FsOpenHook;
		extern EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, UINT, UINT, DWORD64*, DWORD64(*)(DWORD64, DWORD, DWORD64, char*, int), INT16, DWORD64, UINT), DWORD64, UINT, UINT, DWORD64*, DWORD64(*)(DWORD64, DWORD, DWORD64, char*, int), INT16, DWORD64, UINT> MountDataPaksHook;
		extern EGSDK::Utils::Hook::MHook<void*, bool(*)(void*), void*> FsCheckZipCrcHook;
		extern EGSDK::Utils::Hook::MHook<void*, void* (*)(void*), void*> AuthenticateDataAddNewFileHook;
	}
}