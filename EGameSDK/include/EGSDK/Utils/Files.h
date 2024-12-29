#pragma once
#include <filesystem>
#include <string>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace Files {
		extern EGameSDK_API const std::string GetDesktopDir();
		extern EGameSDK_API const std::string GetDocumentsDir();
		extern EGameSDK_API const std::string GetLocalAppDataDir();
		extern EGameSDK_API const std::filesystem::path GetCurrentProcDirectoryFS();
		extern EGameSDK_API const std::string GetCurrentProcDirectory();
		extern EGameSDK_API const bool FileExistsInDir(const char* fileName, const char* dir);
	}
}