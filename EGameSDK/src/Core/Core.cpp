#include <EGSDK\Core\Core.h>
#include <EGSDK\Core\SaveGameManager.h>
#include <EGSDK\GamePH\PlayerHealthModule.h>
#include <EGSDK\GamePH\PlayerInfectionModule.h>
#include <EGSDK\GamePH\PlayerVariables.h>
#include <EGSDK\GamePH\GamePH_Misc.h>
#include <EGSDK\Utils\Hook.h>
#include <EGSDK\Utils\Files.h>
#include <spdlog\spdlog.h>
#include <spdlog\sinks\rotating_file_sink.h>
#include <DbgHelp.h>
#include <thread>

namespace EGSDK::Core {
	std::vector<spdlog::sink_ptr> spdlogSinks{};
	std::counting_semaphore<4> maxHookThreads(4);
	static std::vector<std::thread> threads{};
	static HANDLE keepAliveEvent{};

	int rendererAPI = 0;
	uint32_t gameVer = 0;

	void SetDefaultLoggerSettings(std::shared_ptr<spdlog::logger> logger) {
		logger->flush_on(spdlog::level::trace);
		logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] %^[%l]%$ [%@__%!] %v");
	}
	void InitLogger() {
		constexpr size_t maxSize = static_cast<size_t>(1048576) * 100;
		constexpr size_t maxFiles = 2;

		try {
			spdlogSinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("EGameSDK-log.txt", maxSize, maxFiles, true));

			std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>("EGameSDK", spdlogSinks.begin(), spdlogSinks.end());

			SetDefaultLoggerSettings(logger);
			spdlog::set_default_logger(logger);
		} catch (const std::exception& e) {
			UNREFERENCED_PARAMETER(e);
			std::string errorMsg = "Failed creating spdlog instance, EXCEPTION: " + std::string(e.what()) + "\n\nThis shouldn't happen! Contact developer.";
			MessageBox(nullptr, errorMsg.c_str(), "FATAL GAME ERROR", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
			throw std::runtime_error("Failed creating spdlog instance");
		}
	}

	std::string GetSDKStoragePath() {
		std::string localAppDataDir = Utils::Files::GetLocalAppDataDir();
		if (localAppDataDir.empty()) {
			SPDLOG_ERROR("Could not get local app data directory");
			return {};
		}
		return localAppDataDir += "\\EGameSDK";
	}

	void OnPostUpdate() {
		GamePH::PlayerVariables::GetPlayerVars();
		GamePH::PlayerHealthModule::UpdateClassAddr();
		GamePH::PlayerInfectionModule::UpdateClassAddr();
	}
#ifndef EXCP_HANDLER_DISABLE_DEBUG
	static bool WriteMiniDump(PEXCEPTION_POINTERS pExceptionPointers) {
		HANDLE hFile = CreateFile("EGameSDK-dump.dmp", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
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
	static long WINAPI CrashHandler(PEXCEPTION_POINTERS ExceptionInfo) {
		SPDLOG_ERROR("Crash Handler threw an exception with code {}. Game is exiting, writing mini-dump in the mean time.", ExceptionInfo->ExceptionRecord->ExceptionCode);
		std::string errorMsg = "";

		if (WriteMiniDump(ExceptionInfo)) {
			SPDLOG_INFO("Mini-dump written to \"EGameSDK-dump.dmp\". Please send this to developer for further help!");
			errorMsg = "EGameSDK encountered a fatal error that caused the game to crash.\n\nA file \"" + Utils::Files::GetCurrentProcDirectory() + "\\EGameSDK-dump.dmp\" has been generated. Please send this file to the developer!\n\nThe game will now close once you press OK.";
		} else {
			SPDLOG_ERROR("Failed to write mini-dump.");
			errorMsg = "EGameSDK encountered a fatal error that caused the game to crash.\nEGameSDK failed to generate a crash dump file unfortunately, which means it is harder to find the cause of the crash.\n\nThe game will now close once you press OK.";
		}

		MessageBox(nullptr, errorMsg.c_str(), "FATAL GAME ERROR", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
		exit(0);
	}
#endif
	static void GameVersionCheck() {
		try {
			SPDLOG_DEBUG("Attempting to get current game version");
			gameVer = GamePH::GetCurrentGameVersion();
			SPDLOG_DEBUG("Got game version: {}", gameVer);
		} catch (const std::exception& e) {
			std::string errorMsg = "Failed to get game version, EXCEPTION: " + std::string(e.what()) + "\n\nThis shouldn't happen! Contact developer.";
			SPDLOG_ERROR(errorMsg);
			MessageBox(nullptr, errorMsg.c_str(), "FATAL GAME ERROR", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
			throw std::runtime_error("Failed getting game version");
		}

		SPDLOG_INFO("Got game version: v{}", GamePH::GameVerToStr(gameVer));
		SPDLOG_DEBUG("Comparing game version with compatible versions");
		if (std::find(SUPPORTED_GAME_VERSIONS.begin(), SUPPORTED_GAME_VERSIONS.end(), Core::gameVer) == SUPPORTED_GAME_VERSIONS.end()) {
			std::string warningMsg = "Unsupported game version v" + GamePH::GameVerToStr(gameVer) + ".\n\nPlease note that your game version has not been officially tested with this version of EGameSDK, therefore expect bugs, glitches or other mods to completely stop working.\n\nThe following game versions are officially supported: ";
			for (const auto& version : SUPPORTED_GAME_VERSIONS)
				warningMsg += "- v" + GamePH::GameVerToStr(version) + "\n";

			SPDLOG_WARN(warningMsg);
			std::thread([warningMsg]() {
				MessageBox(nullptr, warningMsg.c_str(), "EGameSDK unsupported game version", MB_ICONWARNING | MB_OK | MB_SETFOREGROUND);
			}).detach();
		}
	}
	uint64_t WINAPI MainThread(HMODULE hModule) {
#ifndef EXCP_HANDLER_DISABLE_DEBUG
		SetUnhandledExceptionFilter(CrashHandler);
#endif

		SPDLOG_INFO("Getting game version");
		GameVersionCheck();

		SPDLOG_INFO("Initializing offsets and patterns");
		OffsetManager::InitializeOffsetsAndPatterns();

		SPDLOG_INFO("Initializing hooks");
		for (auto& hook : (*EGSDK::Utils::Hook::HookBase::GetInstances())[hModule]) {
			threads.emplace_back([&hook]() {
				maxHookThreads.acquire();

				if (hook->IsHooking()) {
					SPDLOG_INFO("Hooking \"{}\"", hook->GetName().data());
					while (hook->IsHooking())
						continue;

					if (hook->IsHooked())
						SPDLOG_INFO("Hooked \"{}\"!", hook->GetName().data());
				} else if (hook->IsHooked())
					SPDLOG_INFO("Hooked \"{}\"!", hook->GetName().data());
				else if (hook->CanHookOnStartup()) {
					SPDLOG_INFO("Hooking \"{}\"", hook->GetName().data());
					if (hook->TryHooking())
						SPDLOG_INFO("Hooked \"{}\"!", hook->GetName().data());
				}

				maxHookThreads.release();
			}).detach();
		}

		SPDLOG_INFO("Sorting Player Variables");
		threads.emplace_back([]() {
			if (GamePH::PlayerVariables::SortPlayerVars())
				SPDLOG_INFO("Player Variables sorted");
			else
				SPDLOG_ERROR("Failed to sort player variables");
		}).detach();

		SPDLOG_INFO("Starting up SaveGameManager");
		threads.emplace_back([]() {
			SaveGameManager::Init();
			SaveGameManager::Loop();
		}).detach();

		SPDLOG_INFO("Creating keepAliveEvent");
		keepAliveEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		if (!keepAliveEvent) {
			SPDLOG_ERROR("Failed to create keepAliveEvent");
			MessageBox(nullptr, "EGameSDK encountered a fatal error: failed to create keepAliveEvent", "Fatal game error", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
			throw std::runtime_error("Failed to create keepAliveEvent");
		}
		WaitForSingleObject(keepAliveEvent, INFINITE);

		for (auto& thread : threads) {
			if (thread.joinable())
				thread.join();
		}

		return true;
	}
	void Cleanup() {
		SPDLOG_INFO("Game requested exit, running cleanup");

		SPDLOG_INFO("Unhooking everything");
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
		SPDLOG_INFO("Unhooked everything");

		SetEvent(keepAliveEvent);
	}
}
