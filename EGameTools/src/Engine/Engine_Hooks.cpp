#include <filesystem>
#include <EGSDK\Utils\Hook.h>
#include <EGSDK\Utils\Time.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\Engine\CBaseCamera.h>
#include <EGSDK\Engine\Engine_Misc.h>
#include <EGSDK\Offsets.h>
#include <EGT\Menu\Camera.h>
#include <EGT\Menu\Misc.h>

namespace EGT::Engine {
	namespace Hooks {
#pragma region MoveCameraFromForwardUpPos
		bool switchedFreeCamByGamePause = false;
		EGSDK::Vector3 freeCamPosBeforeGamePause{};

		static void detourMoveCameraFromForwardUpPos(void*, float* a3, float* a4, EGSDK::Vector3* pos);
		static EGSDK::Utils::Hook::MHook<void*, void(*)(void*, float*, float*, EGSDK::Vector3*)> MoveCameraFromForwardUpPosHook{ "MoveCameraFromForwardUpPos", &EGSDK::Offsets::Get_MoveCameraFromForwardUpPos, &detourMoveCameraFromForwardUpPos };

		static void detourMoveCameraFromForwardUpPos(void* pCBaseCamera, float* a3, float* a4, EGSDK::Vector3* pos) {
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded() || iLevel->IsTimerFrozen())
				return MoveCameraFromForwardUpPosHook.pOriginal(pCBaseCamera, a3, a4, pos);

			if (Menu::Camera::freeCam.GetValue() && switchedFreeCamByGamePause) {
				switchedFreeCamByGamePause = false;
				*pos = freeCamPosBeforeGamePause;
				return MoveCameraFromForwardUpPosHook.pOriginal(pCBaseCamera, a3, a4, pos);
			}
			if (!Menu::Camera::thirdPersonCamera.GetValue() || Menu::Camera::photoMode.GetValue() || Menu::Camera::freeCam.GetValue() || !pos)
				return MoveCameraFromForwardUpPosHook.pOriginal(pCBaseCamera, a3, a4, pos);

			EGSDK::Engine::CBaseCamera* viewCam = static_cast<EGSDK::Engine::CBaseCamera*>(iLevel->GetViewCamera());
			if (!viewCam)
				return MoveCameraFromForwardUpPosHook.pOriginal(pCBaseCamera, a3, a4, pos);

			EGSDK::Vector3 forwardVec{};
			viewCam->GetForwardVector(&forwardVec);
			const EGSDK::Vector3 normForwardVec = forwardVec.normalize();
			EGSDK::Vector3 leftVec{};
			viewCam->GetLeftVector(&leftVec);
			const EGSDK::Vector3 normLeftVec = leftVec.normalize();

			EGSDK::Vector3 newCamPos = *pos - normForwardVec * -Menu::Camera::tpDistanceBehindPlayer;
			newCamPos.Y += Menu::Camera::tpHeightAbovePlayer - 1.5f;
			newCamPos -= normLeftVec * Menu::Camera::tpHorizontalDistanceFromPlayer;

			*pos = newCamPos;

			MoveCameraFromForwardUpPosHook.pOriginal(pCBaseCamera, a3, a4, pos);
		}
#pragma endregion

#pragma region fs::open
		static const std::string userModFilesFullPath = "\\\\?\\" + std::filesystem::absolute("..\\..\\..\\source\\data\\EGameTools\\UserModFiles").string();

		static std::vector<std::string> cachedUserModDirs{};
		static EGSDK::Utils::Time::Timer timeSinceCache{ 0 };
		static void CacheUserModDirs() {
			SPDLOG_INFO("Recaching user mod directories");

			if (!cachedUserModDirs.empty())
				cachedUserModDirs.clear();

			cachedUserModDirs.push_back(userModFilesFullPath);
			try {
				const auto rdi = std::filesystem::recursive_directory_iterator(userModFilesFullPath);
				for (auto entry = std::filesystem::begin(rdi); entry != std::filesystem::end(rdi); ++entry) {
					if (!entry->is_directory())
						continue;

					cachedUserModDirs.push_back(entry->path().string());
				}
			} catch (const std::exception& e) {
				SPDLOG_ERROR("Exception thrown while caching user mod directories: {}", e.what());
			}
		}

		static void* GetFsOpen() {
			return EGSDK::Utils::Memory::GetProcAddr("filesystem_x64_rwdi.dll", "?open@fs@@YAPEAUSFsFile@@V?$string_const@D@ttl@@W4TYPE@EFSMode@@W45FFSOpenFlags@@@Z");
		}
		static DWORD64 detourFsOpen(DWORD64 file, DWORD a2, DWORD a3);
		EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, DWORD, DWORD)> FsOpenHook{ "fs::open", &GetFsOpen, &detourFsOpen };

		static DWORD64 detourFsOpen(DWORD64 file, DWORD a2, DWORD a3) {
			const DWORD64 firstByte = (file >> 56) & 0xFF; // get first byte of addr

			const char* filePath = reinterpret_cast<const char*>(file & 0x1FFFFFFFFFFFFFFF); // remove first byte of addr in case it exists
			const std::string fileName = std::filesystem::path(filePath).filename().string();
			if (fileName.empty() || fileName.contains("EGameTools") || fileName.ends_with(".model"))
				return FsOpenHook.pOriginal(file, a2, a3);

			if (timeSinceCache.DidTimePass()) {
				CacheUserModDirs();
				timeSinceCache = EGSDK::Utils::Time::Timer(5000);
			}

			std::string finalPath{};
			try {
				for (const auto& entry : cachedUserModDirs) {
					finalPath = entry + "\\" + fileName;
					if (!std::filesystem::exists(finalPath))
						continue;
					
					finalPath.erase(0, userModFilesFullPath.size() + 1); // erase entire path until mod folder
					const char* filePath2 = finalPath.c_str();
					SPDLOG_INFO("Loading user mod file \"{}\"", finalPath.c_str());

					DWORD64 finalAddr = reinterpret_cast<DWORD64>(filePath2);
					if (firstByte != 0x0)
						finalAddr |= (firstByte << 56);

					const DWORD64 result = FsOpenHook.pOriginal(finalAddr, a2, a3);
					if (!result) {
						SPDLOG_ERROR("fs::open returned 0! Something went wrong with loading user mod file \"{}\"!\nPlease make sure the path to the file is no longer than 260 characters!", finalPath.c_str());
						return FsOpenHook.pOriginal(file, a2, a3);
					}
					return result;
				}
			} catch (const std::exception& e) {
				SPDLOG_ERROR("Exception thrown while loading user mod file \"{}\": {}", finalPath.c_str(), e.what());
			}
			return FsOpenHook.pOriginal(file, a2, a3);
		}
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
				DWORD64(*pMount)(mount_path*, USHORT, void**) = (decltype(pMount))EGSDK::Utils::Memory::GetProcAddr("filesystem_x64_rwdi.dll", "?mount@fs@@YA_NAEBUmount_path@1@GPEAPEAVCFsMount@@@Z");
				if (!pMount)
					return 0;

				return pMount(path, flags, a3);
			}
		}

		static void* GetCResourceLoadingRuntimeCreate() {
			return EGSDK::Utils::Memory::GetProcAddr("engine_x64_rwdi.dll", "?Create@CResourceLoadingRuntime@@SAPEAV1@_N@Z");
		}
		static void* detourCResourceLoadingRuntimeCreate(bool noTexStreaming);
		EGSDK::Utils::Hook::MHook<void*, void*(*)(bool)> CResourceLoadingRuntimeCreateHook{ "CResourceLoadingRuntimeCreate", &GetCResourceLoadingRuntimeCreate, &detourCResourceLoadingRuntimeCreate };

		static void* detourCResourceLoadingRuntimeCreate(bool noTexStreaming) {
			std::string gamePath = userModFilesFullPath;
			EGSDK::Utils::Values::str_replace(gamePath, "\\ph\\source\\data\\EGameTools\\UserModFiles", "");

			fs::mount_path pathPtr = fs::mount_path();
			pathPtr.gamePath = gamePath.c_str();

			try {
				const auto rdi = std::filesystem::recursive_directory_iterator(userModFilesFullPath);
				for (auto entry = std::filesystem::begin(rdi); entry != std::filesystem::end(rdi); ++entry) {
					if (entry->is_directory())
						continue;
					std::string fullPakPath = entry->path().string();
					if (!EGSDK::Utils::Values::str_ends_with_ci(fullPakPath, ".pak"))
						continue;

					std::string pakPath = fullPakPath;
					pakPath.erase(0, gamePath.size() + 1);

					pathPtr.pakPath = pakPath.c_str();
					pathPtr.fullPakPath = fullPakPath.c_str();

					SPDLOG_INFO("Loading user PAK mod file \"{}\"", pakPath.c_str());
					if (!fs::mount(&pathPtr, 1, nullptr))
						SPDLOG_ERROR("fs::mount returned 0! Something went wrong with loading user PAK mod file \"{}\"!\nPlease make sure the path to the file is no longer than 260 characters, and make sure the file is valid!", pakPath.c_str());
				}
			} catch (const std::exception& e) {
				SPDLOG_ERROR("Exception thrown while iterating over user mod directories for PAK loading: {}", e.what());
			}

			return CResourceLoadingRuntimeCreateHook.pOriginal(noTexStreaming);
		}
#pragma endregion

#pragma region MountDataPaks
		int mountDataPaksRanWith8Count = 0;

		static DWORD64 detourMountDataPaks(DWORD64 a1, UINT a2, UINT a3, DWORD64* a4, DWORD64(*a5)(DWORD64, DWORD, DWORD64, char*, int), INT16 a6, DWORD64 a7, UINT a8);
		EGSDK::Utils::Hook::MHook<void*, DWORD64(*)(DWORD64, UINT, UINT, DWORD64*, DWORD64(*)(DWORD64, DWORD, DWORD64, char*, int), INT16, DWORD64, UINT)> MountDataPaksHook{ "MountDataPaks", &EGSDK::Offsets::Get_MountDataPaks, &detourMountDataPaks };

		static DWORD64 detourMountDataPaks(DWORD64 a1, UINT a2, UINT a3, DWORD64* a4, DWORD64(*a5)(DWORD64, DWORD, DWORD64, char*, int), INT16 a6, DWORD64 a7, UINT a8) {
			if (Menu::Misc::increaseDataPAKsLimit.GetValue()) {
				if (a8 == 8)
					mountDataPaksRanWith8Count++;

				a8 = 200;
			}
			return MountDataPaksHook.pOriginal(a1, a2, a3, a4, a5, a6, a7, a8);
		}
#pragma endregion

#pragma region FsCheckZipCrc
		static void* GetFsCheckZipCrc() {
			return EGSDK::Utils::Memory::GetProcAddr("filesystem_x64_rwdi.dll", "?check_zip_crc@izipped_buffer_file@fs@@QEAA_NXZ");
		}
		static bool detourFsCheckZipCrc(void* instance);
		EGSDK::Utils::Hook::MHook<void*, bool(*)(void*)> FsCheckZipCrcHook{ "FsCheckZipCrc", &GetFsCheckZipCrc, &detourFsCheckZipCrc };

		static bool detourFsCheckZipCrc(void* instance) {
			return Menu::Misc::disableSavegameCRCCheck.GetValue() ? true : FsCheckZipCrcHook.pOriginal(instance);
		}
#pragma endregion

#pragma region AuthenticateDataAddNewFile
		static void* GetAuthenticateDataAddNewFile() {
			return EGSDK::Utils::Memory::GetProcAddr("engine_x64_rwdi.dll", "?AddNewFile@Results@AuthenticateData@@QEAAAEAVFile@12@XZ");
		}
		static void* detourAuthenticateDataAddNewFile(void* instance);
		EGSDK::Utils::Hook::MHook<void*, void*(*)(void*)> AuthenticateDataAddNewFileHook{ "AuthenticateDataAddNewFile", &GetAuthenticateDataAddNewFile, &detourAuthenticateDataAddNewFile };

		static void* detourAuthenticateDataAddNewFile(void* instance) {
			void* result = AuthenticateDataAddNewFileHook.pOriginal(instance);
			if (Menu::Misc::disableDataPAKsCRCCheck.GetValue())
				EGSDK::Engine::AuthenticateDataResultsClear(instance);
			return result;
		}
#pragma endregion
	}
}