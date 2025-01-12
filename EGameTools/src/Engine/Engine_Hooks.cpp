#include <filesystem>
#include <EGSDK\Utils\Time.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\Engine\CBaseCamera.h>
#include <EGSDK\Engine\Engine_Misc.h>
#include <EGSDK\Offsets.h>
#include <EGT\Engine\Engine_Hooks.h>
#include <EGT\Menu\Camera.h>
#include <EGT\Menu\Misc.h>

namespace EGT::Engine {
	namespace Hooks {
#pragma region MoveCameraFromForwardUpPos
		bool switchedFreeCamByGamePause = false;
		EGSDK::Vector3 freeCamPosBeforeGamePause{};

		static EGSDK::Utils::Hook::MHook<void*, void(*)(void*, float*, float*, EGSDK::Vector3*), void*, float*, float*, EGSDK::Vector3*> MoveCameraFromForwardUpPosHook{ "MoveCameraFromForwardUpPos", &EGSDK::Offsets::Get_MoveCameraFromForwardUpPos, [](void* pCBaseCamera, float* a3, float* a4, EGSDK::Vector3* pos) -> void {
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded() || iLevel->IsTimerFrozen())
				return MoveCameraFromForwardUpPosHook.ExecuteCallbacksWithOriginal(pCBaseCamera, a3, a4, pos);
			if (!pos)
				return;

			if (Menu::Camera::freeCam.GetValue() && switchedFreeCamByGamePause) {
				switchedFreeCamByGamePause = false;
				*pos = freeCamPosBeforeGamePause;
				return MoveCameraFromForwardUpPosHook.ExecuteCallbacksWithOriginal(pCBaseCamera, a3, a4, pos);
			}
			if ((!Menu::Camera::thirdPersonCamera.GetValue() && Menu::Camera::cameraOffset.isDefault()) || Menu::Camera::photoMode.GetValue() || Menu::Camera::freeCam.GetValue())
				return MoveCameraFromForwardUpPosHook.ExecuteCallbacksWithOriginal(pCBaseCamera, a3, a4, pos);

			auto viewCam = static_cast<EGSDK::Engine::CBaseCamera*>(iLevel->GetViewCamera());
			if (!viewCam)
				return MoveCameraFromForwardUpPosHook.ExecuteCallbacksWithOriginal(pCBaseCamera, a3, a4, pos);

			EGSDK::Vector3 forwardVec, upVec, leftVec = {};
			viewCam->GetForwardVector(&forwardVec);
			viewCam->GetUpVector(&upVec);
			viewCam->GetLeftVector(&leftVec);

			const auto normForwardVec = forwardVec.normalize();
			const auto normUpVec = upVec.normalize();
			const auto normLeftVec = leftVec.normalize();

			EGSDK::Vector3 newCamPos = *pos;

			if (!Menu::Camera::cameraOffset.isDefault() && !Menu::Camera::thirdPersonCamera.GetValue()) {
				newCamPos -= normLeftVec * Menu::Camera::cameraOffset.X;
				newCamPos.Y += Menu::Camera::cameraOffset.Y;
				newCamPos -= normForwardVec * Menu::Camera::cameraOffset.Z;
			} else if (Menu::Camera::thirdPersonCamera.GetValue()) {
				newCamPos -= normForwardVec * -Menu::Camera::thirdPersonDistanceBehindPlayer;
				newCamPos.Y += Menu::Camera::thirdPersonHeightAbovePlayer - 1.5f;
				newCamPos -= normLeftVec * Menu::Camera::thirdPersonHorizontalDistanceFromPlayer;
			}

			*pos = newCamPos;
			MoveCameraFromForwardUpPosHook.ExecuteCallbacksWithOriginal(pCBaseCamera, a3, a4, pos);
		} };
#pragma endregion

#pragma region SetFOV
		static void* GetSetFOV() {
			return EGSDK::Utils::Memory::GetProcAddr("engine_x64_rwdi.dll", "?SetFOV@IBaseCamera@@QEAAXM@Z");
		}
		static EGSDK::Utils::Hook::MHook<void*, void(*)(void*, float), void*, float> SetFOVHook{ "SetFOV", &GetSetFOV, [](void* pCBaseCamera, float fov) -> void {
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return SetFOVHook.ExecuteCallbacksWithOriginal(pCBaseCamera, fov);
			if (Menu::Camera::thirdPersonCamera.GetValue())
				fov = static_cast<float>(Menu::Camera::thirdPersonFOV);
			else if (!Menu::Camera::firstPersonZoomIn.IsKeyDown())
				Menu::Camera::originalFirstPersonFOVAfterZoomIn = fov;

			return SetFOVHook.ExecuteCallbacksWithOriginal(pCBaseCamera, fov);
		} };
#pragma endregion

#pragma region fs::open
		static const std::string userModFilesFullPath = "\\\\?\\" + std::filesystem::absolute("..\\..\\..\\source\\data\\EGameTools\\UserModFiles").string();
		static std::vector<std::string> cachedUserModDirs{};
		static EGSDK::Utils::Time::Timer cacheUpdateTimer{ 0 };

		static void RecacheUserModDirectories() {
			SPDLOG_INFO("Recaching user mod directories");
			
			try {
				cachedUserModDirs.clear();
				cachedUserModDirs.push_back(userModFilesFullPath);

				for (const auto& entry : std::filesystem::recursive_directory_iterator(userModFilesFullPath)) {
					if (entry.is_directory())
						cachedUserModDirs.push_back(entry.path().string());
				}
			} catch (const std::exception& e) {
				SPDLOG_ERROR("Exception thrown while caching user mod directories: {}", e.what());
			}
		}

		static std::optional<std::string> FindValidModFilePath(const std::string& fileName) {
			for (const auto& dir : cachedUserModDirs) {
				std::string potentialPath = dir + "\\" + fileName;
				if (std::filesystem::exists(potentialPath)) {
					SPDLOG_INFO("Found user mod file: {}", potentialPath);
					return potentialPath.substr(userModFilesFullPath.size() + 1); // Strip base path
				}
			}
			return std::nullopt;
		}

		static void* GetFsOpen() {
			return EGSDK::Utils::Memory::GetProcAddr("filesystem_x64_rwdi.dll", "?open@fs@@YAPEAUSFsFile@@V?$string_const@D@ttl@@W4TYPE@EFSMode@@W45FFSOpenFlags@@@Z");
		}
		EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, DWORD, DWORD), DWORD64, DWORD, DWORD> FsOpenHook{ "fs::open", &GetFsOpen, [](DWORD64 file, DWORD a2, DWORD a3) -> DWORD64 {
			const DWORD64 firstByte = (file >> 56) & 0xFF; // get first byte of addr
			const char* filePath = reinterpret_cast<const char*>(file & 0x1FFFFFFFFFFFFFFF); // remove first byte of addr in case it exists
			const std::string fileName = std::filesystem::path(filePath).filename().string();
			if (fileName.empty() || fileName.contains("EGameTools") || fileName.ends_with(".model"))
				return FsOpenHook.ExecuteCallbacksWithOriginal(file, a2, a3);

			if (cacheUpdateTimer.DidTimePass()) {
				RecacheUserModDirectories();
				cacheUpdateTimer = EGSDK::Utils::Time::Timer(5000);
			}

			std::string finalPath{};
			try {
				if (auto modFilePath = FindValidModFilePath(fileName)) {
					const char* modFileCStr = modFilePath->c_str();
					SPDLOG_INFO("Loading user mod file: {}", modFilePath->c_str());

					DWORD64 modFileAddr = reinterpret_cast<DWORD64>(modFileCStr);
					if (firstByte != 0x0)
						modFileAddr |= (firstByte << 56);

					DWORD64 result = FsOpenHook.ExecuteCallbacksWithOriginal(modFileAddr, a2, a3);
					if (!result) {
						SPDLOG_ERROR("fs::open returned 0! Something went wrong with loading user mod file \"{}\"!\nPlease make sure the path to the file is no longer than 260 characters!", finalPath.c_str());
						return FsOpenHook.ExecuteCallbacksWithOriginal(file, a2, a3);
					}
				}
			} catch (const std::exception& e) {
				SPDLOG_ERROR("Exception thrown while loading user mod file \"{}\": {}", finalPath.c_str(), e.what());
			}
			return FsOpenHook.ExecuteCallbacksWithOriginal(file, a2, a3);
		} };
#pragma endregion

		// Thank you @12brendon34 on Discord for help with finding the function responsible for .PAK loading!
#pragma region CResourceLoadingRuntimeCreate
		namespace fs {
			struct mount_path {
				union {
					const char* gamePath;
					EGSDK::ClassHelpers::buffer<0x8, const char*> pakPath;
					EGSDK::ClassHelpers::buffer<0x10, const char*> fullPakPath;
				};
			};

			static DWORD64 mount(mount_path* path, USHORT flags, void** a3) {
				return EGSDK::Utils::Memory::SafeCallFunction<DWORD64>("filesystem_x64_rwdi.dll", "?mount@fs@@YA_NAEBUmount_path@1@GPEAPEAVCFsMount@@@Z", 0, path, flags, a3);
			}
		}

		static bool MountUserPakFile(const std::string& fullPakPath, const std::string& gamePath) {
			try {
				std::string pakPath = fullPakPath;
				pakPath.erase(0, gamePath.size() + 1);

				fs::mount_path path{};
				path.gamePath = gamePath.c_str();
				path.pakPath = pakPath.c_str();
				path.fullPakPath = fullPakPath.c_str();

				SPDLOG_INFO("Attempting to load user PAK mod file: \"{}\"", pakPath);

				// Attempt to mount the PAK file
				if (!fs::mount(&path, 1, nullptr)) {
					SPDLOG_ERROR("Failed to mount user PAK mod file \"{}\". Ensure the file is valid and the path is no longer than 260 characters.", pakPath);
					return false;
				}

				SPDLOG_INFO("Successfully loaded user PAK mod file: \"{}\"", pakPath);
				return true;
			} catch (const std::exception& e) {
				SPDLOG_ERROR("Error mounting user PAK mod file \"{}\": {}", fullPakPath, e.what());
				return false;
			}
		}
		static void IterateAndMountUserPakFiles(const std::string& userModFilesPath, const std::string& gamePath) {
			try {
				for (const auto& entry : std::filesystem::recursive_directory_iterator(userModFilesPath)) {
					if (entry.is_directory())
						continue;

					const std::string fullPakPath = entry.path().string();
					if (EGSDK::Utils::Values::str_ends_with_ci(fullPakPath, ".pak"))
						MountUserPakFile(fullPakPath, gamePath);
				}
			} catch (const std::exception& e) {
				SPDLOG_ERROR("Exception during PAK file iteration: {}", e.what());
			}
		}

		static void* GetCResourceLoadingRuntimeCreate() {
			return EGSDK::Utils::Memory::GetProcAddr("engine_x64_rwdi.dll", "?Create@CResourceLoadingRuntime@@SAPEAV1@_N@Z");
		}
		static EGSDK::Utils::Hook::MHook<void*, void* (*)(bool), bool> CResourceLoadingRuntimeCreateHook{ "CResourceLoadingRuntimeCreate", &GetCResourceLoadingRuntimeCreate, [](bool noTexStreaming) -> void* {
			std::string gamePath = userModFilesFullPath;
			EGSDK::Utils::Values::str_replace(gamePath, "\\ph\\source\\data\\EGameTools\\UserModFiles", "");

			SPDLOG_INFO("Mounting user PAK mod files from: {}", userModFilesFullPath);
			IterateAndMountUserPakFiles(userModFilesFullPath, gamePath);

			return CResourceLoadingRuntimeCreateHook.ExecuteCallbacksWithOriginal(noTexStreaming);
		} };
#pragma endregion

#pragma region MountDataPaks
		int mountDataPaksRanWith8Count = 0;

		EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, UINT, UINT, DWORD64*, DWORD64(*)(DWORD64, DWORD, DWORD64, char*, int), INT16, DWORD64, UINT), DWORD64, UINT, UINT, DWORD64*, DWORD64(*)(DWORD64, DWORD, DWORD64, char*, int), INT16, DWORD64, UINT> MountDataPaksHook{ "MountDataPaks", &EGSDK::Offsets::Get_MountDataPaks, [](DWORD64 a1, UINT a2, UINT a3, DWORD64* a4, DWORD64(*a5)(DWORD64, DWORD, DWORD64, char*, int), INT16 a6, DWORD64 a7, UINT a8) -> DWORD64 {
			if (Menu::Misc::increaseDataPAKsLimit.GetValue()) {
				if (a8 == 8)
					mountDataPaksRanWith8Count++;
				a8 = 200;
			}
			return MountDataPaksHook.pOriginal(a1, a2, a3, a4, a5, a6, a7, a8);
		} };
#pragma endregion

#pragma region FsCheckZipCrc
		static void* GetFsCheckZipCrc() {
			return EGSDK::Utils::Memory::GetProcAddr("filesystem_x64_rwdi.dll", "?check_zip_crc@izipped_buffer_file@fs@@QEAA_NXZ");
		}
		EGSDK::Utils::Hook::MHook<void*, bool(*)(void*), void*> FsCheckZipCrcHook{ "FsCheckZipCrc", &GetFsCheckZipCrc, [](void* instance) -> bool {
			return Menu::Misc::disableSavegameCRCCheck.GetValue() ? true : FsCheckZipCrcHook.ExecuteCallbacksWithOriginal(instance);
		} };
#pragma endregion

#pragma region AuthenticateDataAddNewFile
		static void* GetAuthenticateDataAddNewFile() {
			return EGSDK::Utils::Memory::GetProcAddr("engine_x64_rwdi.dll", "?AddNewFile@Results@AuthenticateData@@QEAAAEAVFile@12@XZ");
		}
		EGSDK::Utils::Hook::MHook<void*, void* (*)(void*), void*> AuthenticateDataAddNewFileHook{ "AuthenticateDataAddNewFile", &GetAuthenticateDataAddNewFile, [](void* instance) -> void* {
			void* result = AuthenticateDataAddNewFileHook.ExecuteCallbacksWithOriginal(instance);
			if (Menu::Misc::disableDataPAKsCRCCheck.GetValue())
				EGSDK::Engine::AuthenticateDataResultsClear(instance);
			return result;
		} };
#pragma endregion
	}
}