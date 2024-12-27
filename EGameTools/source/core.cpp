#include <pch.h>
#include "config\config.h"
#include "core.h"
#include "game\Engine\engine_hooks.h"
#include "game\GamePH\LevelDI.h"
#include "game\GamePH\PlayerHealthModule.h"
#include "game\GamePH\PlayerInfectionModule.h"
#include "game\GamePH\PlayerVariables.h"
#include "game\GamePH\gameph_misc.h"
#include "menu\menu.h"
#include "menu\misc.h"

#pragma region KeyBindOption
bool KeyBindOption::wasAnyKeyPressed = false;
bool KeyBindOption::scrolledMouseWheelUp = false;
bool KeyBindOption::scrolledMouseWheelDown = false;
#pragma endregion

namespace Core {
#pragma region Console
	static void DisableConsoleQuickEdit() {
		DWORD prev_mode = 0;
		const HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
		GetConsoleMode(hInput, &prev_mode);
		SetConsoleMode(hInput, prev_mode & ENABLE_EXTENDED_FLAGS);
	}

	static FILE* f = nullptr;
	void EnableConsole() {
		AllocConsole();
		SetConsoleTitle("EGameTools");
		freopen_s(&f, "CONOUT$", "w", stdout);
		DisableConsoleQuickEdit();
	}
	void DisableConsole() {
		if (f)
			fclose(f);
		FreeConsole();
	}
#pragma endregion

	// Core
	std::atomic<bool> exiting = false;
	static std::vector<std::thread> threads{};
	static HANDLE keepAliveEvent{};

	int rendererAPI = 0;
	DWORD gameVer = 0;

	std::counting_semaphore<4> maxHookThreads(4);

	static bool LoopHookRenderer() {
		SPDLOG_INFO("Entering LoopHookRenderer loop");
		while (true) {
			if (exiting) {
				SPDLOG_INFO("Exiting LoopHookRenderer loop due to exiting flag");
				return false;
			}

			Sleep(1000);

			if (!rendererAPI) {
				SPDLOG_WARN("rendererAPI is null, skipping iteration");
				continue;
			}
			kiero::Status::Enum initStatus = kiero::init(rendererAPI == 11 ? kiero::RenderType::D3D11 : kiero::RenderType::D3D12);
			if (initStatus != kiero::Status::Success) {
				//SPDLOG_ERROR("kiero::init failed with status {}", initStatus);
				continue;
			}

			SPDLOG_INFO("kiero::init successful");

			switch (kiero::getRenderType()) {
			case kiero::RenderType::D3D11:
				SPDLOG_INFO("Initializing D3D11");
				impl::d3d11::init();
				break;
			case kiero::RenderType::D3D12:
				SPDLOG_INFO("Initializing D3D12");
				impl::d3d12::init();
				break;
			default:
				SPDLOG_WARN("Unknown render type");
				break;
			}

			break;
		}
		SPDLOG_INFO("Exiting LoopHookRenderer loop normally");
		return true;
	}

	static bool WarnMsgSeenFileExists() {
		SPDLOG_DEBUG("Checking if WarnMsgBoxSeen file exists");
		try {
			const std::string localAppDataDir = Utils::Files::GetLocalAppDataDir();
			SPDLOG_DEBUG("Local App Data Dir: {}", localAppDataDir);
			if (localAppDataDir.empty()) {
				SPDLOG_WARN("Local App Data Dir is empty");
				return false;
			}
			const std::string finalPath = std::string(localAppDataDir) + "\\EGameTools\\" + "WarnMsgBoxSeen";
			SPDLOG_DEBUG("Final Path: {}", finalPath);

			bool exists = std::filesystem::exists(finalPath);
			SPDLOG_DEBUG("WarnMsgBoxSeen file exists: {}", exists);
			return exists;
		} catch (const std::exception& e) {
			SPDLOG_ERROR("Exception thrown while trying to check if WarnMsgBoxSeen file exists: {}", e.what());
			return false;
		}
	}
	static void CreateWarnMsgSeenFile() {
		SPDLOG_INFO("Creating WarnMsgBoxSeen file...");
		try {
			const std::string localAppDataDir = Utils::Files::GetLocalAppDataDir();
			SPDLOG_DEBUG("Local App Data Dir: {}", localAppDataDir);
			if (localAppDataDir.empty()) {
				SPDLOG_WARN("Local App Data Dir is empty, skipping WarnMsgBoxSeen file creation");
				return;
			}
			const std::string dirPath = std::string(localAppDataDir) + "\\EGameTools\\";
			SPDLOG_DEBUG("Directory Path: {}", dirPath);
			std::filesystem::create_directories(dirPath);
			SPDLOG_INFO("Created directory: {}", dirPath);

			const std::string finalPath = dirPath + "WarnMsgBoxSeen";
			SPDLOG_DEBUG("Final Path: {}", finalPath);
			if (!std::filesystem::exists(finalPath)) {
				SPDLOG_INFO("Creating WarnMsgBoxSeen file...");
				std::ofstream outFile(finalPath.c_str(), std::ios::binary);
				if (!outFile.is_open()) {
					SPDLOG_ERROR("Failed to open WarnMsgBoxSeen file for writing");
					return;
				}
				outFile.close();
				SPDLOG_INFO("WarnMsgBoxSeen file created successfully");
			}
			SPDLOG_INFO("WarnMsgBoxSeen file already exists, skipping creation");
		} catch (const std::exception& e) {
			SPDLOG_ERROR("Exception thrown while trying to create WarnMsgBoxSeen file: {}", e.what());
		}
	}
	static void CreateSymlinkForLoadingFiles() {
		SPDLOG_DEBUG("Entering CreateSymlinkForLoadingFiles");
		try {
			const char* userModFilesPath = "..\\..\\..\\source\\data\\EGameTools\\UserModFiles";
			const char* eGameToolsPath = "..\\..\\..\\source\\data\\EGameTools";

			SPDLOG_DEBUG("UserModFilesPath: {}", userModFilesPath);
			SPDLOG_DEBUG("EGameToolsPath: {}", eGameToolsPath);

			if (!std::filesystem::exists(userModFilesPath)) {
				SPDLOG_DEBUG("UserModFilesPath does not exist, creating directories");
				std::filesystem::create_directories(userModFilesPath);
				SPDLOG_INFO("Created directories: {}", userModFilesPath);
			} else
				SPDLOG_DEBUG("UserModFilesPath already exists");

			for (const auto& entry : std::filesystem::directory_iterator(".")) {
				SPDLOG_DEBUG("Iterating directory entry: {}", entry.path().filename().string());

				if (entry.path().filename().string() == "EGameTools") {
					SPDLOG_DEBUG("Found EGameTools directory");

					if (is_symlink(entry.symlink_status()) && std::filesystem::equivalent("EGameTools", eGameToolsPath)) {
						SPDLOG_DEBUG("EGameTools is already a symlink, returning");
						return;
					}

					SPDLOG_DEBUG("Removing existing EGameTools directory");
					std::filesystem::remove(entry.path());
					SPDLOG_INFO("Removed directory: {}", entry.path().filename().string());
				}
			}

			SPDLOG_WARN("Creating symlink \"EGameTools\" for \"Dying Light 2\\ph\\source\\data\\EGameTools\" folder");

			std::string symlinkPath = Utils::Files::GetCurrentProcDirectory() + "\\EGameTools";
			SPDLOG_DEBUG("SymlinkPath: {}", symlinkPath);

			std::filesystem::create_directory_symlink(eGameToolsPath, symlinkPath);
			SPDLOG_INFO("Game shortcut created: {}", symlinkPath);
		} catch (const std::exception& e) {
			SPDLOG_ERROR("Exception thrown while trying to create folder shortcut: {}", e.what());
			SPDLOG_WARN("This error should NOT affect any features of my mod. The shortcut is only a way for the user to easily access the folder \"Dying Light 2\\ph\\source\\data\\EGameTools\".");

			if (WarnMsgSeenFileExists()) {
				SPDLOG_DEBUG("WarnMsgSeenFile already exists, returning");
				return;
			}

			std::thread([]() {
				int msgBoxResult = MessageBoxA(nullptr, "EGameTools has failed creating a folder shortcut \"EGameTools\" inside \"Dying Light 2\\ph\\work\\bin\\x64\".\n\nTo fix this, please open Windows Settings and, for Windows 11, go to System -> For developers and enable \"Developer Mode\", or for Windows 10, go to Update & Security -> For developers and enable \"Developer Mode\".\nAfter doing this, restart the game and there should be no issues with shortcut creation anymore.\n\nIf the above solution doesn't work, then in order to install mods inside the \"UserModFiles\" folder, please manually navigate to \"Dying Light 2\\ph\\source\\data\\EGameTools\\UserModFiles\".\n\nAlternatively, run the game once as administrator from the exe and once a shortcut has been created, close the game and open up the game from Steam or whatever platform you're using.\n\nDo you want to continue seeing this warning message every game launch?", "Error creating EGameTools folder shortcut", MB_ICONWARNING | MB_YESNO | MB_SETFOREGROUND);

				switch (msgBoxResult) {
				case IDNO:
					SPDLOG_DEBUG("User chose to not see warning message again");
					CreateWarnMsgSeenFile();
					break;
				default:
					SPDLOG_DEBUG("User chose to see warning message again");
					break;
				}
			}).detach();
		}
		SPDLOG_DEBUG("Exiting CreateSymlinkForLoadingFiles");
	}

	void InitLogger() {
		constexpr size_t maxSize = static_cast<size_t>(1048576) * 100;
		constexpr size_t maxFiles = 10;

		try {
			static std::vector<spdlog::sink_ptr> sinks{};
			sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("..\\..\\..\\source\\data\\EGameTools\\log.txt", maxSize, maxFiles, true));
			sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());

			std::shared_ptr<spdlog::logger> combined_logger = std::make_shared<spdlog::logger>("EGameTools", std::begin(sinks), std::end(sinks));
			combined_logger->flush_on(spdlog::level::trace);
			combined_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] %^[%l]%$ [%@__%!] %v");

			spdlog::set_default_logger(combined_logger);
		} catch (const std::exception& e) {
			UNREFERENCED_PARAMETER(e);
			std::cout << "Failed creating spdlog instance! Please contact mod author, this is not supposed to happen!" << std::endl;
			std::cout << "Game will exit in 30 seconds. Close this window once you've read the text." << std::endl;
			Sleep(30000);
			exit(0);
		}
	}

	void OnPostUpdate() {
		GamePH::PlayerVariables::GetPlayerVars();

		for (auto& menuTab : *Menu::MenuTab::GetInstances())
			menuTab.second->Update();

		GamePH::PlayerHealthModule::UpdateClassAddr();
		GamePH::PlayerInfectionModule::UpdateClassAddr();

		static bool mountDataPaksErrorShown = false;
		if (!mountDataPaksErrorShown && Engine::Hooks::mountDataPaksRanWith8Count < 3 && Menu::Misc::increaseDataPAKsLimit.GetValue() && GamePH::PlayerVariables::Get()) {
			SPDLOG_ERROR("MountDataPaks hook ran less than 3 times with the data PAKs limit set to 8. This means the increased data PAKs limit might not work correctly! If this error message appears and your data PAKs past \"data7.pak\" have not loaded, please contact author.");
			mountDataPaksErrorShown = true;
		}
	}
#ifndef EXCP_HANDLER_DISABLE_DEBUG
	static bool WriteMiniDump(PEXCEPTION_POINTERS pExceptionPointers) {
		HANDLE hFile = CreateFileA("EGameTools-dump.dmp", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		MINIDUMP_EXCEPTION_INFORMATION mdei{};
		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = pExceptionPointers;
		mdei.ClientPointers = false;

		int success = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, (pExceptionPointers ? &mdei : nullptr), nullptr, nullptr);
		CloseHandle(hFile);

		return success;
	}
	static long WINAPI CrashHandler(PEXCEPTION_POINTERS exceptionInfo) {
		SPDLOG_ERROR("Crash Handler threw an exception with code {}. Game is exiting, writing mini-dump in the mean time.", exceptionInfo->ExceptionRecord->ExceptionCode);
		std::string errorMsg = "";

		if (WriteMiniDump(exceptionInfo)) {
			SPDLOG_INFO("Mini-dump written to \"EGameTools-dump.dmp\". Please send this to mod author for further help!");
			errorMsg = "EGameTools encountered a fatal error that caused the game to crash.\n\nA file \"" + Utils::Files::GetCurrentProcDirectory() + "\\EGameTools-dump.dmp\" has been generated. Please send this file to the author of the mod!\n\nThe game will now close once you press OK.";
		} else {
			SPDLOG_ERROR("Failed to write mini-dump.");
			errorMsg = "EGameTools encountered a fatal error that caused the game to crash.\n\nEGameTools failed to generate a crash dump file unfortunately, which means it is harder to find the cause of the crash.\n\nThe game will now close once you press OK.";
		}

		MessageBoxA(nullptr, errorMsg.c_str(), "Fatal game error", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
		exit(0);
	}
#endif
	static void GameVersionCheck() {
		try {
			SPDLOG_DEBUG("Attempting to get current game version");
			gameVer = GamePH::GetCurrentGameVersion();
			SPDLOG_DEBUG("Got game version: {}", gameVer);
		} catch (const std::exception& e) {
			SPDLOG_ERROR("Failed to get game version, EXCEPTION: {}", e.what());
			SPDLOG_ERROR("This shouldn't happen! Contact developer.");
			SPDLOG_DEBUG("Exiting GameVersionCheck function due to exception");
			return;
		}

		SPDLOG_INFO("Got game version: v{}", GamePH::GameVerToStr(gameVer));
		SPDLOG_DEBUG("Comparing game version with compatible version");
		if (Core::gameVer != GAME_VER_COMPAT) {
			SPDLOG_WARN("Game version is not compatible with mod");
			SPDLOG_ERROR("Please note that your game version has not been officially tested with this mod, therefore expect bugs, glitches or the mod to completely stop working. If so, please {}", Core::gameVer > GAME_VER_COMPAT ? "wait for a new patch." : "upgrade your game version to one that the mod supports.");
		} else
			SPDLOG_DEBUG("Game version is compatible with mod");
	}
	DWORD64 WINAPI MainThread(HMODULE hModule) {
#ifndef EXCP_HANDLER_DISABLE_DEBUG
		SetUnhandledExceptionFilter(CrashHandler);
#endif

		SPDLOG_WARN("Getting game version");
		GameVersionCheck();

		SPDLOG_WARN("Initializing config");
		Config::InitConfig();
		threads.emplace_back(Config::ConfigLoop);

		SPDLOG_WARN("Creating symlink for loading files");
		CreateSymlinkForLoadingFiles();

		SPDLOG_INFO("Initializing hooks");
		for (auto& hook : *Utils::Hook::HookBase::GetInstances()) {
			threads.emplace_back([&hook]() {
				maxHookThreads.acquire();

				if (hook->isHooking) {
					SPDLOG_WARN("Hooking \"{}\"", hook->name.data());
					while (hook->isHooking)
						Sleep(10);

					if (hook->isHooked)
						SPDLOG_INFO("Hooked \"{}\"!", hook->name.data());
				} else if (hook->isHooked)
					SPDLOG_INFO("Hooked \"{}\"!", hook->name.data());
				else {
					SPDLOG_WARN("Hooking \"{}\"", hook->name.data());
					if (hook->HookLoop())
						SPDLOG_INFO("Hooked \"{}\"!", hook->name.data());
				}

				maxHookThreads.release();
			}).detach();
		}
		
		SPDLOG_WARN("Sorting Player Variables");
		threads.emplace_back([]() {
			if (GamePH::PlayerVariables::SortPlayerVars())
				SPDLOG_INFO("Player Variables sorted");
			else
				SPDLOG_ERROR("Failed to sort player variables");
		}).detach();

		SPDLOG_WARN("Hooking DX11/DX12 renderer");
		threads.emplace_back([]() {
			if (LoopHookRenderer())
				SPDLOG_INFO("Hooked \"DX11/DX12 renderer\"!");
			else
				SPDLOG_ERROR("Failed to hook renderer");
		}).detach();

		SPDLOG_INFO("Creating keepAliveEvent");
		keepAliveEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
		if (!keepAliveEvent) {
			SPDLOG_ERROR("Failed to create keepAliveEvent");
			MessageBoxA(nullptr, "EGameTools encountered a fatal error: failed to create keepAliveEvent", "Fatal game error", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
			exit(0);
		}
		WaitForSingleObject(keepAliveEvent, INFINITE);

		for (auto& thread : threads) {
			if (thread.joinable())
				thread.join();
		}

		return true;
	}

	void Cleanup() {
		exiting = true;

		SPDLOG_WARN("Game requested exit, running cleanup");
		SPDLOG_WARN("Saving config to file");
		Config::SaveConfig();
		SPDLOG_INFO("Config saved to file");

		SPDLOG_WARN("Unhooking everything");
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
		SPDLOG_INFO("Unhooked everything");

		SetEvent(keepAliveEvent);
	}
}
