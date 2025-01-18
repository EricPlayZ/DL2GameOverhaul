#pragma once
#include <filesystem>
#include <EGSDK\Exports.h>
#include <EGSDK\Utils\Time.h>

namespace EGSDK::Core {
	class EGameSDK_API SaveGameManager {
    public:
        static void Init();
        static void Loop();
    private:
        static void PerformBackup();

        static void InitializeBackupSlot();
        static std::filesystem::path DetectSaveGamePathSteam();
        static std::filesystem::path DetectSaveGamePathEpic();

        static std::filesystem::path saveGamePath;
        static std::filesystem::path backupRootPath;
        static int maxBackupSlots;
        static std::chrono::minutes backupInterval;
        static int currentBackupSlot;

        static Utils::Time::Timer timeSpentInitializing;
        static bool initialized;
        static bool isInitializing;
        static bool triedInitializingWithSteam;
        static bool loopIsRunning;
	};
}