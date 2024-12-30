#include <spdlog\spdlog.h>
#include <spdlog\sinks\rotating_file_sink.h>
#include <kiero\kiero.h>
#include <EGSDK\Core\Core.h>
#include <EGSDK\Utils\Files.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\GamePH\PlayerHealthModule.h>
#include <EGSDK\GamePH\PlayerInfectionModule.h>
#include <EGSDK\GamePH\PlayerVariables.h>
#include <EGSDK\GamePH\GamePH_Hooks.h>
#include <EGSDK\GamePH\GamePH_Misc.h>
#include <EGT\ImGui_impl\D3D11_impl.h>
#include <EGT\ImGui_impl\D3D12_impl.h>
#include <EGT\Config\Config.h>
#include <EGT\Engine\Engine_Hooks.h>
#include <EGT\Menu\Menu.h>
#include <EGT\Menu\Misc.h>
#include <DbgHelp.h>
#include <thread>
#include <semaphore>
#include <fstream>
#include <iostream>

namespace EGT::Core {
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

	std::counting_semaphore<4> maxHookThreads(4);

	static bool LoopHookRenderer() {
		SPDLOG_INFO("Entering LoopHookRenderer loop");
		while (true) {
			if (exiting) {
				SPDLOG_INFO("Exiting LoopHookRenderer loop due to exiting flag");
				return false;
			}

			Sleep(1000);

			if (!EGSDK::Core::rendererAPI) {
				SPDLOG_WARN("rendererAPI is null, skipping iteration");
				continue;
			}
			kiero::Status::Enum initStatus = kiero::init(EGSDK::Core::rendererAPI == 11 ? kiero::RenderType::D3D11 : kiero::RenderType::D3D12);
			if (initStatus != kiero::Status::Success) {
				//SPDLOG_ERROR("kiero::init failed with status {}", initStatus);
				continue;
			}

			SPDLOG_INFO("kiero::init successful");

			switch (kiero::getRenderType()) {
			case kiero::RenderType::D3D11:
				SPDLOG_INFO("Initializing D3D11 ImGui implementation");
				EGT::ImGui_impl::D3D11::Init();
				break;
			case kiero::RenderType::D3D12:
				SPDLOG_INFO("Initializing D3D12 ImGui implementation");
				EGT::ImGui_impl::D3D12::Init();
				break;
			default:
				SPDLOG_WARN("Unknown renderer type");
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
			const std::string localAppDataDir = EGSDK::Utils::Files::GetLocalAppDataDir();
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
			const std::string localAppDataDir = EGSDK::Utils::Files::GetLocalAppDataDir();
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

			std::string symlinkPath = EGSDK::Utils::Files::GetCurrentProcDirectory() + "\\EGameTools";
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
		try {
			static std::vector<spdlog::sink_ptr> sinks{};
			EGSDK::Core::spdlogSinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());

			std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>("EGameTools", std::begin(EGSDK::Core::spdlogSinks), std::end(EGSDK::Core::spdlogSinks));

			EGSDK::Core::SetDefaultLoggerSettings(logger);
			spdlog::set_default_logger(logger);
		} catch (const std::exception& e) {
			UNREFERENCED_PARAMETER(e);
			std::string errorMsg = "Failed creating spdlog instance, EXCEPTION: " + std::string(e.what()) + "\n\nThis shouldn't happen! Contact developer.";
			MessageBoxA(nullptr, errorMsg.c_str(), "FATAL GAME ERROR", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
			exit(0);
		}
	}

	static void OnPostUpdate(void* pGameDI_PH2) {
		for (auto& menuTab : *Menu::MenuTab::GetInstances())
			menuTab.second->Update();

		static bool mountDataPaksErrorShown = false;
		if (!mountDataPaksErrorShown && EGT::Engine::Hooks::mountDataPaksRanWith8Count < 3 && Menu::Misc::increaseDataPAKsLimit.GetValue() && EGSDK::GamePH::PlayerVariables::Get()) {
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
			errorMsg = "EGameTools encountered a fatal error that caused the game to crash.\n\nA file \"" + EGSDK::Utils::Files::GetCurrentProcDirectory() + "\\EGameTools-dump.dmp\" has been generated. Please send this file to the author of the mod!\n\nThe game will now close once you press OK.";
		} else {
			SPDLOG_ERROR("Failed to write mini-dump.");
			errorMsg = "EGameTools encountered a fatal error that caused the game to crash.\n\nEGameTools failed to generate a crash dump file unfortunately, which means it is harder to find the cause of the crash.\n\nThe game will now close once you press OK.";
		}

		MessageBoxA(nullptr, errorMsg.c_str(), "Fatal game error", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
		exit(0);
	}
#endif
	static void GameVersionCheck() {
		SPDLOG_INFO("Game version: v{}", EGSDK::GamePH::GameVerToStr(EGSDK::Core::gameVer));
		SPDLOG_DEBUG("Comparing game version with compatible version");
		if (EGSDK::Core::gameVer != GAME_VER_COMPAT) {
			SPDLOG_WARN("Game version is not compatible with mod");
			SPDLOG_ERROR("Please note that your game version has not been officially tested with this mod, therefore expect bugs, glitches or the mod to completely stop working. If so, please {}", EGSDK::Core::gameVer > GAME_VER_COMPAT ? "wait for a new patch." : "upgrade your game version to one that the mod supports.");
		} else
			SPDLOG_DEBUG("Game version is compatible with mod");
	}
	DWORD64 WINAPI MainThread(HMODULE hModule) {
		EGSDK::GamePH::Hooks::OnPostUpdateHook.RegisterCallback(OnPostUpdate);

#ifndef EXCP_HANDLER_DISABLE_DEBUG
		SetUnhandledExceptionFilter(CrashHandler);
#endif

		SPDLOG_WARN("Running game version checks");
		GameVersionCheck();

		SPDLOG_WARN("Initializing config");
		Config::InitConfig();
		threads.emplace_back(Config::ConfigLoop);

		SPDLOG_WARN("Creating symlink for loading files");
		CreateSymlinkForLoadingFiles();

		SPDLOG_INFO("Initializing hooks");
		for (auto& hook : (*EGSDK::Utils::Hook::HookBase::GetInstances())[hModule]) {
			threads.emplace_back([&hook]() {
				maxHookThreads.acquire();

				if (hook->IsHooking()) {
					SPDLOG_WARN("Hooking \"{}\"", hook->GetName().data());
					while (hook->IsHooking())
						Sleep(10);

					if (hook->IsHooked())
						SPDLOG_INFO("Hooked \"{}\"!", hook->GetName().data());
				} else if (hook->IsHooked())
					SPDLOG_INFO("Hooked \"{}\"!", hook->GetName().data());
				else {
					SPDLOG_WARN("Hooking \"{}\"", hook->GetName().data());
					if (hook->HookLoop())
						SPDLOG_INFO("Hooked \"{}\"!", hook->GetName().data());
				}

				maxHookThreads.release();
			}).detach();
		}

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
