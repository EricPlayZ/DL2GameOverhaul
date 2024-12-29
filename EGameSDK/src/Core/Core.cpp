#include <EGSDK\Core\Core.h>
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
#include <vector>
#include <semaphore>

namespace EGSDK::Core {
	std::atomic<bool> exiting = false;
	std::vector<spdlog::sink_ptr> spdlogSinks{};
	static std::vector<std::thread> threads{};
	static HANDLE keepAliveEvent{};

	int rendererAPI = 0;
	DWORD gameVer = 0;

	std::counting_semaphore<4> maxHookThreads(4);

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
			MessageBoxA(nullptr, errorMsg.c_str(), "FATAL GAME ERROR", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
			exit(0);
		}
	}

	void OnPostUpdate() {
		GamePH::PlayerVariables::GetPlayerVars();
		GamePH::PlayerHealthModule::UpdateClassAddr();
		GamePH::PlayerInfectionModule::UpdateClassAddr();
	}
#ifndef EXCP_HANDLER_DISABLE_DEBUG
	static bool WriteMiniDump(PEXCEPTION_POINTERS pExceptionPointers) {
		HANDLE hFile = CreateFileA("EGameSDK-dump.dmp", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
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
			errorMsg = "EGameSDK encountered a fatal error that caused the game to crash.\n\EGameSDK failed to generate a crash dump file unfortunately, which means it is harder to find the cause of the crash.\n\nThe game will now close once you press OK.";
		}

		MessageBoxA(nullptr, errorMsg.c_str(), "FATAL GAME ERROR", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
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
			MessageBoxA(nullptr, errorMsg.c_str(), "FATAL GAME ERROR", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
			exit(0);
		}

		SPDLOG_INFO("Got game version: v{}", GamePH::GameVerToStr(gameVer));
		SPDLOG_DEBUG("Comparing game version with compatible version");
		if (Core::gameVer != GAME_VER_COMPAT) {
			std::string errorMsg = "Unsupported game version v" + GamePH::GameVerToStr(gameVer) + ".\n\nPlease note that your game version has not been officially tested with EGameSDK, therefore expect bugs, glitches or other mods to completely stop working.";
			SPDLOG_ERROR(errorMsg);
			MessageBoxA(nullptr, errorMsg.c_str(), "EGameSDK unsupported game version", MB_ICONWARNING | MB_OK | MB_SETFOREGROUND);
		}
	}
	DWORD64 WINAPI MainThread(HMODULE hModule) {
#ifndef EXCP_HANDLER_DISABLE_DEBUG
		SetUnhandledExceptionFilter(CrashHandler);
#endif

		SPDLOG_WARN("Getting game version");
		GameVersionCheck();

		SPDLOG_WARN("Initializing hooks");
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

		SPDLOG_INFO("Creating keepAliveEvent");
		keepAliveEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
		if (!keepAliveEvent) {
			SPDLOG_ERROR("Failed to create keepAliveEvent");
			MessageBoxA(nullptr, "EGameSDK encountered a fatal error: failed to create keepAliveEvent", "Fatal game error", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
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

		SPDLOG_WARN("Unhooking everything");
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
		SPDLOG_INFO("Unhooked everything");

		SetEvent(keepAliveEvent);
	}
}
