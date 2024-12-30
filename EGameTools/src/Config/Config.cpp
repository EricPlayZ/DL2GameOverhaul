#include <Windows.h>
#include <any>
#include <filesystem>
#include <spdlog\spdlog.h>
#include <ini\ini.h>
#include <EGSDK\Utils\Files.h>
#include <EGT\Menu\Camera.h>
#include <EGT\Menu\Menu.h>
#include <EGT\Menu\Misc.h>
#include <EGT\Menu\Player.h>
#include <EGT\Menu\Teleport.h>
#include <EGT\Menu\Weapon.h>
#include <EGT\Menu\World.h>
#include <EGT\Config\Config.h>

namespace EGT::Config {
	enum ValueType {
		OPTION,
		Float,
		String
	};

	struct VKey {
		constexpr VKey(std::string_view name, int code) : name(name), code(code) {}

		std::string_view name;
		int code;
	};
	static constexpr auto virtualKeyCodes = std::to_array<VKey>({
		{ "VK_NONE", VK_NONE },

		// Function keys
		{ "VK_F1", VK_F1 },
		{ "VK_F2", VK_F2 },
		{ "VK_F3", VK_F3 },
		{ "VK_F4", VK_F4 },
		{ "VK_F5", VK_F5 },
		{ "VK_F6", VK_F6 },
		{ "VK_F7", VK_F7 },
		{ "VK_F8", VK_F8 },
		{ "VK_F9", VK_F9 },
		{ "VK_F10", VK_F10 },
		{ "VK_F11", VK_F11 },
		{ "VK_F12", VK_F12 },

		// Number keys
		{ "VK_0", '0' },
		{ "VK_1", '1' },
		{ "VK_2", '2' },
		{ "VK_3", '3' },
		{ "VK_4", '4' },
		{ "VK_5", '5' },
		{ "VK_6", '6' },
		{ "VK_7", '7' },
		{ "VK_8", '8' },
		{ "VK_9", '9' },
		{ "0", '0' },
		{ "1", '1' },
		{ "2", '2' },
		{ "3", '3' },
		{ "4", '4' },
		{ "5", '5' },
		{ "6", '6' },
		{ "7", '7' },
		{ "8", '8' },
		{ "9", '9' },

		// Alphabetic keys
		{ "VK_A", 'A' },
		{ "VK_B", 'B' },
		{ "VK_C", 'C' },
		{ "VK_D", 'D' },
		{ "VK_E", 'E' },
		{ "VK_F", 'F' },
		{ "VK_G", 'G' },
		{ "VK_H", 'H' },
		{ "VK_I", 'I' },
		{ "VK_J", 'J' },
		{ "VK_K", 'K' },
		{ "VK_L", 'L' },
		{ "VK_M", 'M' },
		{ "VK_N", 'N' },
		{ "VK_O", 'O' },
		{ "VK_P", 'P' },
		{ "VK_Q", 'Q' },
		{ "VK_R", 'R' },
		{ "VK_S", 'S' },
		{ "VK_T", 'T' },
		{ "VK_U", 'U' },
		{ "VK_V", 'V' },
		{ "VK_W", 'W' },
		{ "VK_X", 'X' },
		{ "VK_Y", 'Y' },
		{ "VK_Z", 'Z' },
		{ "A", 'A' },
		{ "B", 'B' },
		{ "C", 'C' },
		{ "D", 'D' },
		{ "E", 'E' },
		{ "F", 'F' },
		{ "G", 'G' },
		{ "H", 'H' },
		{ "I", 'I' },
		{ "J", 'J' },
		{ "K", 'K' },
		{ "L", 'L' },
		{ "M", 'M' },
		{ "N", 'N' },
		{ "O", 'O' },
		{ "P", 'P' },
		{ "Q", 'Q' },
		{ "R", 'R' },
		{ "S", 'S' },
		{ "T", 'T' },
		{ "U", 'U' },
		{ "V", 'V' },
		{ "W", 'W' },
		{ "X", 'X' },
		{ "Y", 'Y' },
		{ "Z", 'Z' },

		// Special keys
		{"VK_BACK", VK_BACK },
		{"VK_TAB", VK_TAB },
		{"VK_RETURN", VK_RETURN },
		{"VK_CAPITAL", VK_CAPITAL },
		{"VK_SPACE", VK_SPACE },
		{"VK_PRIOR", VK_PRIOR },
		{"VK_NEXT", VK_NEXT },
		{"VK_END", VK_END },
		{"VK_HOME", VK_HOME },
		{"VK_LEFT", VK_LEFT },
		{"VK_UP", VK_UP },
		{"VK_RIGHT", VK_RIGHT },
		{"VK_DOWN", VK_DOWN },
		{"VK_INSERT", VK_INSERT },
		{"VK_DELETE", VK_DELETE },

		// Numpad keys
		{ "VK_NUMPAD0", VK_NUMPAD0 },
		{ "VK_NUMPAD1", VK_NUMPAD1 },
		{ "VK_NUMPAD2", VK_NUMPAD2 },
		{ "VK_NUMPAD3", VK_NUMPAD3 },
		{ "VK_NUMPAD4", VK_NUMPAD4 },
		{ "VK_NUMPAD5", VK_NUMPAD5 },
		{ "VK_NUMPAD6", VK_NUMPAD6 },
		{ "VK_NUMPAD7", VK_NUMPAD7 },
		{ "VK_NUMPAD8", VK_NUMPAD8 },
		{ "VK_NUMPAD9", VK_NUMPAD9 },
		{ "VK_MULTIPLY", VK_MULTIPLY },
		{ "VK_ADD", VK_ADD },
		{ "VK_SUBTRACT", VK_SUBTRACT },
		{ "VK_DECIMAL", VK_DECIMAL },
		{ "VK_DIVIDE", VK_DIVIDE },

		// Modifier keys
		{ "VK_SHIFT", VK_LSHIFT },
		{ "VK_LSHIFT", VK_LSHIFT },
		{ "VK_RSHIFT", VK_RSHIFT },
		{ "VK_CONTROL", VK_LCONTROL },
		{ "VK_LCONTROL", VK_LCONTROL },
		{ "VK_RCONTROL", VK_RCONTROL },
		{ "VK_MENU", VK_LMENU },
		{ "VK_LMENU", VK_LMENU },
		{ "VK_RMENU", VK_RMENU },

		// Other keys
		{ "VK_OEM_1", VK_OEM_1 },
		{ "VK_OEM_PLUS", VK_OEM_PLUS },
		{ "VK_OEM_COMMA", VK_OEM_COMMA },
		{ "VK_OEM_MINUS", VK_OEM_MINUS },
		{ "VK_OEM_PERIOD", VK_OEM_PERIOD },
		{ "VK_OEM_2", VK_OEM_2 },
		{ "VK_OEM_3", VK_OEM_3 },
		{ "VK_OEM_4", VK_OEM_4 },
		{ "VK_OEM_5", VK_OEM_5 },
		{ "VK_OEM_6", VK_OEM_6 },
		{ "VK_OEM_7", VK_OEM_7 }
	});
	
	struct ConfigEntry {
		std::string_view section;
		std::string_view key;
		std::any value;
		void* optionPtr;
		ValueType type;
	};
	static const auto configVariablesDefault = std::to_array<ConfigEntry>({
		{ "Menu", "Opacity", 99.0f, &Menu::opacity, Float },
		{ "Menu", "Scale", 1.0f, &Menu::scale, Float },
		{ "Menu", "FirstTimeRunning", true, &Menu::firstTimeRunning, OPTION },
		{ "Menu", "HasSeenChangelog", false, &Menu::hasSeenChangelog, OPTION },
		{ "Menu:Keybinds", "MenuToggleKey", std::string("VK_F5"), &Menu::menuToggle, String},
		{ "Menu:Keybinds", "GodModeToggleKey", std::string("VK_F6"), &Menu::Player::godMode, String},
		{ "Menu:Keybinds", "FreezePlayerToggleKey", std::string("VK_NONE"), &Menu::Player::freezePlayer, String },
		{ "Menu:Keybinds", "UnlimitedImmunityToggleKey", std::string("VK_NONE"), &Menu::Player::unlimitedImmunity, String},
		{ "Menu:Keybinds", "UnlimitedStaminaToggleKey", std::string("VK_NONE"), &Menu::Player::unlimitedStamina, String},
		{ "Menu:Keybinds", "UnlimitedItemsToggleKey", std::string("VK_NONE"), &Menu::Player::unlimitedItems, String},
		{ "Menu:Keybinds", "OneHitKillToggleKey", std::string("VK_NONE"), &Menu::Player::oneHitKill, String},
		{ "Menu:Keybinds", "DisableOutOfBoundsTimerToggleKey", std::string("VK_NONE"), &Menu::Player::disableOutOfBoundsTimer, String},
		{ "Menu:Keybinds", "NightrunnerModeToggleKey", std::string("VK_F7"), &Menu::Player::nightrunnerMode, String},
		{ "Menu:Keybinds", "OneHandedModeToggleKey", std::string("VK_NONE"), &Menu::Player::oneHandedMode, String},
		{ "Menu:Keybinds", "AllowGrappleHookInSafezoneToggleKey", std::string("VK_NONE"), &Menu::Player::allowGrappleHookInSafezone, String},
		{ "Menu:Keybinds", "DisableAirControlToggleKey", std::string("VK_NONE"), &Menu::Player::disableAirControl, String },
		{ "Menu:Keybinds", "UnlimitedDurabilityToggleKey", std::string("VK_NONE"), &Menu::Weapon::unlimitedDurability, String },
		{ "Menu:Keybinds", "UnlimitedAmmoToggleKey", std::string("VK_NONE"), &Menu::Weapon::unlimitedAmmo, String },
		{ "Menu:Keybinds", "NoSpreadToggleKey", std::string("VK_NONE"), &Menu::Weapon::noSpread, String },
		{ "Menu:Keybinds", "NoRecoilToggleKey", std::string("VK_NONE"), &Menu::Weapon::noRecoil, String },
		{ "Menu:Keybinds", "InstantReloadToggleKey", std::string("VK_NONE"), &Menu::Weapon::instantReload, String },
		{ "Menu:Keybinds", "FreeCamToggleKey", std::string("VK_F3"), &Menu::Camera::freeCam, String},
		{ "Menu:Keybinds", "TeleportPlayerToCameraToggleKey", std::string("VK_F4"), &Menu::Camera::teleportPlayerToCamera, String},
		{ "Menu:Keybinds", "ThirdPersonToggleKey", std::string("VK_F1"), &Menu::Camera::thirdPersonCamera, String},
		{ "Menu:Keybinds", "UseTPPModelToggleKey", std::string("VK_F2"), &Menu::Camera::tpUseTPPModel, String},
		{ "Menu:Keybinds", "GoProMode", std::string("VK_NONE"), &Menu::Camera::goProMode, String },
		{ "Menu:Keybinds", "DisableSafezoneFOVReduction", std::string("VK_NONE"), &Menu::Camera::disableSafezoneFOVReduction, String },
		{ "Menu:Keybinds", "DisablePhotoModeLimits", std::string("VK_NONE"), &Menu::Camera::disablePhotoModeLimits, String},
		{ "Menu:Keybinds", "DisableHeadCorrectionToggleKey", std::string("VK_NONE"), &Menu::Camera::disableHeadCorrection, String },
		{ "Menu:Keybinds", "TeleportToSelectedLocationToggleKey", std::string("VK_F9"), &Menu::Teleport::teleportToSelectedLocation, String },
		{ "Menu:Keybinds", "TeleportToCoordsToggleKey", std::string("VK_NONE"), &Menu::Teleport::teleportToCoords, String },
		{ "Menu:Keybinds", "DisableHUDToggleKey", std::string("VK_F8"), &Menu::Misc::disableHUD, String},
		{ "Menu:Keybinds", "DisableGamePauseWhileAFKToggleKey", std::string("VK_NONE"), &Menu::Misc::disableGamePauseWhileAFK, String},
		{ "Menu:Keybinds", "FreezeTimeToggleKey", std::string("VK_NONE"), &Menu::World::freezeTime, String},
		{ "Menu:Keybinds", "SlowMotionToggleKey", std::string("VK_4"), &Menu::World::slowMotion, String},
		{ "Player:Misc", "GodMode", false, &Menu::Player::godMode, OPTION },
		{ "Player:Misc", "UnlimitedImmunity", false, &Menu::Player::unlimitedImmunity, OPTION },
		{ "Player:Misc", "UnlimitedStamina", false, &Menu::Player::unlimitedStamina, OPTION },
		{ "Player:Misc", "UnlimitedItems", false, &Menu::Player::unlimitedItems, OPTION },
		{ "Player:Misc", "OneHitKill", false, &Menu::Player::oneHitKill, OPTION },
		{ "Player:Misc", "InvisibleToEnemies", false, &Menu::Player::invisibleToEnemies, OPTION },
		{ "Player:Misc", "DisableOutOfBoundsTimer", true, &Menu::Player::disableOutOfBoundsTimer, OPTION },
		{ "Player:Misc", "NightrunnerMode", false, &Menu::Player::nightrunnerMode, OPTION },
		{ "Player:Misc", "OneHandedMode", false, &Menu::Player::oneHandedMode, OPTION },
		{ "Player:Misc", "AllowGrappleHookInSafezone", false, &Menu::Player::allowGrappleHookInSafezone, OPTION },
		{ "Player:PlayerJumpParameters", "DisableAirControl", false, &Menu::Player::disableAirControl, OPTION },
		{ "Player:PlayerVariables", "Enabled", false, &Menu::Player::playerVariables, OPTION },
		{ "Player:PlayerVariables", "LastSaveSCRPath", std::string(), &Menu::Player::saveSCRPath, String },
		{ "Player:PlayerVariables", "LastLoadSCRFilePath", std::string(), &Menu::Player::loadSCRFilePath, String },
		{ "Weapon:Misc", "UnlimitedDurability", false, &Menu::Weapon::unlimitedDurability, OPTION },
		{ "Weapon:Misc", "UnlimitedAmmo", false, &Menu::Weapon::unlimitedAmmo, OPTION },
		{ "Weapon:Misc", "NoSpread", false, &Menu::Weapon::noSpread, OPTION },
		{ "Weapon:Misc", "NoRecoil", false, &Menu::Weapon::noRecoil, OPTION },
		{ "Weapon:Misc", "InstantReload", false, &Menu::Weapon::instantReload, OPTION },
		{ "Camera:FreeCam", "Speed", 2.0f, &Menu::Camera::freeCamSpeed, Float },
		{ "Camera:FreeCam", "TeleportPlayerToCamera", false, &Menu::Camera::teleportPlayerToCamera, OPTION },
		{ "Camera:ThirdPerson", "Enabled", false, &Menu::Camera::thirdPersonCamera, OPTION },
		{ "Camera:ThirdPerson", "UseTPPModel", true, &Menu::Camera::tpUseTPPModel, OPTION },
		{ "Camera:ThirdPerson", "DistanceBehindPlayer", 2.0f, &Menu::Camera::tpDistanceBehindPlayer, Float },
		{ "Camera:ThirdPerson", "HeightAbovePlayer", 1.35f, &Menu::Camera::tpHeightAbovePlayer, Float },
		{ "Camera:ThirdPerson", "HorizontalDistanceFromPlayer", 0.0f, &Menu::Camera::tpHorizontalDistanceFromPlayer, Float },
		{ "Camera:Misc", "LensDistortion", 20.0f, &Menu::Camera::lensDistortion, Float },
		{ "Camera:Misc", "GoProMode", false, &Menu::Camera::goProMode, OPTION },
		{ "Camera:Misc", "DisableSafezoneFOVReduction", true, &Menu::Camera::disableSafezoneFOVReduction, OPTION },
		{ "Camera:Misc", "DisablePhotoModeLimits", true, &Menu::Camera::disablePhotoModeLimits, OPTION },
		{ "Camera:Misc", "DisableHeadCorrection", false, &Menu::Camera::disableHeadCorrection, OPTION },
		{ "Teleport:SavedLocations", "SavedTeleportLocations", std::string("Bazaar - Highest Point:1944,123.6,932.8;Bazaar - Main Entrance:1962.9,50.1,927.9;Colonel Williams Stronghold - Main Entrance Bridge:994.3,22.8,-1138.6;Dynamo Cars Factory - Main Entrance:2295.9,-2.1,-78.6;Fish Eye - Player Safehouse:1180.4,32.4,-146.8;Fish Eye - Top of The Baloon:1122.6,98.8,-101.2;Observatory - Meeting Room:1951.2,-13.4,-329.6;Observatory - The 2 Domes (No Chemicals):1985.4,19.9,-357.2;Out of Bounds - Cut Road Quest:2693.3,-4.7,-241.5;PK Metro Station - Main Entrance:1886.9,50,628.9;PK Ship - Main Entrance:801.8,4.2,139.8;St. Paul Cathedral - GRE Entrance:463.4,4.2,-421;Tolga & Fatin Quest - Underground Loot Box:2343.9,12.2,-661.5;VNC Tower - \"V\" Logo:1434.2,4.3,-319.3;VNC Tower - Highest Player Safehouse:1424.7,354.6,-455;VNC Tower - Highest Point:1403.8,446.7,-389.8;X13 - Tunnel Near the City Walls Towards Facility:2407.9,36.2,-461.7;X13 - Underground Facility:2437.8,12.2,-649.9;X13 - Waltz Arena:2551.9,15.3,-569.1"), &Menu::Teleport::savedTeleportLocations, String},
		{ "Misc:Misc", "DisableGamePauseWhileAFK", true, &Menu::Misc::disableGamePauseWhileAFK, OPTION },
		{ "Misc:GameChecks", "DisableSavegameCRCCheck", true, &Menu::Misc::disableSavegameCRCCheck, OPTION },
		{ "Misc:GameChecks", "DisableDataPAKsCRCCheck", true, &Menu::Misc::disableDataPAKsCRCCheck, OPTION },
		{ "Misc:GameChecks", "IncreaseDataPAKsLimit", true, &Menu::Misc::increaseDataPAKsLimit, OPTION },
		{ "World:Time", "SlowMotionSpeed", 0.4f, &Menu::World::slowMotionSpeed, Float },
		{ "World:Time", "SlowMotionTransitionTime", 1.0f, &Menu::World::slowMotionTransitionTime, Float }
	});
	std::vector<ConfigEntry> configVariables(configVariablesDefault.begin(), configVariablesDefault.end());
	static constexpr const char* configFileName = "EGameTools.ini";
	static std::filesystem::file_time_type configPreviousWriteTime{};
	static std::filesystem::file_time_type configLastWriteTime{};

	static bool savedConfig = false;

	static inih::INIReader reader{};

	static void UpdateEntry(const ConfigEntry& entry) {
		switch (entry.type) {
		case OPTION:
			reader.UpdateEntry(entry.section.data(), entry.key.data(), std::any_cast<bool>(entry.value));
			break;
		case Float:
			reader.UpdateEntry(entry.section.data(), entry.key.data(), std::any_cast<float>(entry.value));
			break;
		case String:
			reader.UpdateEntry(entry.section.data(), entry.key.data(), std::any_cast<std::string>(entry.value));
			break;
		}
	}

	static void LoadDefaultConfig() {
		reader = inih::INIReader();

		const std::filesystem::path desktopPath = EGSDK::Utils::Files::GetDesktopDir();
		std::string desktopPathStr = desktopPath.string();
		if (!desktopPath.empty() && !(std::filesystem::is_directory(desktopPath.parent_path()) && std::filesystem::is_directory(desktopPath)))
			desktopPathStr = {};

		const std::string loadSCRFilePath = desktopPathStr.empty() ? "" : desktopPathStr + "\\player_variables.scr";

		std::string strValue{};
		for (auto entry : configVariablesDefault) {
			switch (entry.type) {
			case OPTION: {
				ImGui::Option* option = reinterpret_cast<ImGui::Option*>(entry.optionPtr);
				if (option->GetChangesAreDisabled())
					break;
				option->SetBothValues(std::any_cast<bool>(entry.value));
				reader.InsertEntry(entry.section.data(), entry.key.data(), std::any_cast<bool>(entry.value));
				break;
			}
			case Float:
				*reinterpret_cast<float*>(entry.optionPtr) = std::any_cast<float>(entry.value);
				reader.InsertEntry(entry.section.data(), entry.key.data(), std::any_cast<float>(entry.value));
				break;
			case String:
				strValue = std::any_cast<std::string>(entry.value);

				if (entry.key == "LastSaveSCRPath") {
					entry.value = desktopPathStr;
					Menu::Player::saveSCRPath = desktopPathStr;
				} else if (entry.key == "LastLoadSCRFilePath") {
					entry.value = loadSCRFilePath;
					Menu::Player::loadSCRFilePath = loadSCRFilePath;
				} else if (entry.section == "Menu:Keybinds") {
					const std::string toggleKey = strValue;
					if (const auto it = std::ranges::find(virtualKeyCodes, toggleKey, &Config::VKey::name); it != virtualKeyCodes.end())
						reinterpret_cast<ImGui::KeyBindOption*>(entry.optionPtr)->ChangeKeyBind(it->code);
				} else if (entry.key == "SavedTeleportLocations") {
					Menu::Teleport::savedTeleportLocations = Menu::Teleport::ParseTeleportLocations(strValue);
					Menu::Teleport::UpdateTeleportLocationVisualNames();
					break;
				}

				reader.InsertEntry(entry.section.data(), entry.key.data(), strValue);
				break;
			}
		}
	}
	static void LoadAndWriteDefaultConfig() {
		LoadDefaultConfig();
		try {
			inih::INIWriter writer{};
			writer.write(configFileName, reader);
		} catch (const std::runtime_error& e) {
			SPDLOG_ERROR("Error writing file {}: {}", configFileName, e.what());
		}
	}
	static bool ConfigExists() {
		return std::filesystem::exists(configFileName);
	}
	static void CreateConfig() {
		SPDLOG_WARN("{} does not exist (will create now); using default config values", configFileName);
		LoadAndWriteDefaultConfig();
	}
	static void ReadConfig(const bool configUpdate = false) {
		try {
			reader = inih::INIReader(configFileName);

			const std::filesystem::path desktopPath = EGSDK::Utils::Files::GetDesktopDir();
			std::string desktopPathStr = desktopPath.string();
			if (!desktopPath.empty() && !(std::filesystem::is_directory(desktopPath.parent_path()) && std::filesystem::is_directory(desktopPath)))
				desktopPathStr = {};

			const std::string loadSCRFilePathStr = desktopPathStr.empty() ? "" : desktopPathStr + "\\player_variables.scr";

			std::string strValue{};
			for (auto& entry : configVariablesDefault) {
				switch (entry.type) {
				case OPTION: {
					ImGui::Option* option = reinterpret_cast<ImGui::Option*>(entry.optionPtr);
					if (option->GetChangesAreDisabled())
						break;
					option->SetBothValues(reader.Get(entry.section.data(), entry.key.data(), std::any_cast<bool>(entry.value)));
					break;
				}
				case Float:
					*reinterpret_cast<float*>(entry.optionPtr) = reader.Get(entry.section.data(), entry.key.data(), std::any_cast<float>(entry.value));
					break;
				case String:
					strValue = reader.Get(entry.section.data(), entry.key.data(), std::any_cast<std::string>(entry.value));
					if (entry.section == "Menu:Keybinds") {
						if (const auto it = std::ranges::find(virtualKeyCodes, strValue, &Config::VKey::name); it != virtualKeyCodes.end())
							reinterpret_cast<ImGui::KeyBindOption*>(entry.optionPtr)->ChangeKeyBind(it->code);
						break;
					} else if (entry.key == "LastSaveSCRPath") {
						Menu::Player::saveSCRPath = strValue;
						if (Menu::Player::saveSCRPath.empty())
							Menu::Player::saveSCRPath = desktopPathStr;

						const std::filesystem::path saveSCRPath = Menu::Player::saveSCRPath;
						if (!saveSCRPath.empty() && !(std::filesystem::is_directory(saveSCRPath.parent_path()) && std::filesystem::is_directory(saveSCRPath)))
							Menu::Player::saveSCRPath = {};
						break;
					} else if (entry.key == "LastLoadSCRFilePath") {
						Menu::Player::loadSCRFilePath = strValue;
						if (Menu::Player::loadSCRFilePath.empty())
							Menu::Player::loadSCRFilePath = loadSCRFilePathStr;
						const std::filesystem::path loadSCRFilePath = Menu::Player::loadSCRFilePath;

						if (!loadSCRFilePath.empty() && !std::filesystem::is_directory(loadSCRFilePath.parent_path()))
							Menu::Player::loadSCRFilePath = {};
						break;
					} else if (entry.key == "SavedTeleportLocations") {
						Menu::Teleport::savedTeleportLocations = Menu::Teleport::ParseTeleportLocations(strValue.empty() ? std::any_cast<std::string>(entry.value) : strValue);
						Menu::Teleport::UpdateTeleportLocationVisualNames();
						break;
					}

					*reinterpret_cast<std::string*>(entry.optionPtr) = strValue;
					break;
				}
			}

			SPDLOG_INFO(configUpdate ? "Successfully read updated config!" : "Successfully read config!");
		} catch (const std::runtime_error& e) {
			SPDLOG_ERROR("Error writing file {}; using default config values: {}", configFileName, e.what());
			LoadDefaultConfig();
		}
	}

	static bool CheckForChangesInMem() {
		try {
			reader = inih::INIReader(configFileName);

			std::string strValue{};
			for (auto& entry : configVariablesDefault) {
				switch (entry.type) {
				case OPTION:
					if (reader.Get(entry.section.data(), entry.key.data(), std::any_cast<bool>(entry.value)) != reinterpret_cast<ImGui::Option*>(entry.optionPtr)->GetValue())
						return true;
					break;
				case Float:
					if (!EGSDK::Utils::Values::are_samef(reader.Get(entry.section.data(), entry.key.data(), std::any_cast<float>(entry.value)), *reinterpret_cast<float*>(entry.optionPtr)))
						return true;
					break;
				case String:
					strValue = reader.Get(entry.section.data(), entry.key.data(), std::any_cast<std::string>(entry.value));

					if (entry.section == "Menu:Keybinds") {
						ImGui::KeyBindOption* option = reinterpret_cast<ImGui::KeyBindOption*>(entry.optionPtr);

						const auto itConfigVal = std::ranges::find(virtualKeyCodes, strValue, &Config::VKey::name);
						const auto itMemVal = std::ranges::find(virtualKeyCodes, option->GetKeyBind(), &Config::VKey::code);
						if (itConfigVal == virtualKeyCodes.end() || itMemVal == virtualKeyCodes.end())
							break;

						if (strValue != itMemVal->name)
							return true;
						break;
					} else if (entry.key == "SavedTeleportLocations") {
						std::string currentTPLocations = Menu::Teleport::ConvertTeleportLocationsToStr(Menu::Teleport::savedTeleportLocations);
						if (strValue != currentTPLocations)
							return true;
						break;
					}

					if (strValue != *reinterpret_cast<std::string*>(entry.optionPtr))
						return true;
					break;
				}
			}
			return false;
		} catch (const std::runtime_error& e) {
			SPDLOG_ERROR("Error checking for changes in memory: {}", e.what());
			return false;
		}
	}
	void SaveConfig() {
		for (auto& entry : configVariables) {
			switch (entry.type) {
			case OPTION:
				entry.value = reinterpret_cast<ImGui::Option*>(entry.optionPtr)->GetValue();
				break;
			case Float:
				entry.value = *reinterpret_cast<float*>(entry.optionPtr);
				break;
			case String:
				if (entry.section == "Menu:Keybinds") {
					ImGui::KeyBindOption* option = reinterpret_cast<ImGui::KeyBindOption*>(entry.optionPtr);

					if (const auto it = std::ranges::find(virtualKeyCodes, option->GetKeyBind(), &Config::VKey::code); it != virtualKeyCodes.end())
						entry.value = std::string(it->name);
					break;
				} else if (entry.key == "SavedTeleportLocations") {
					entry.value = Menu::Teleport::ConvertTeleportLocationsToStr(Menu::Teleport::savedTeleportLocations);
					break;
				}

				entry.value = *reinterpret_cast<std::string*>(entry.optionPtr);
				break;
			}

			UpdateEntry(entry);
		}

		try {
			inih::INIWriter writer{};
			writer.write(configFileName, reader);

			SPDLOG_INFO("Successfully updated config!");

			savedConfig = true;
		} catch (const std::runtime_error& e) {
			SPDLOG_ERROR("Error saving to file {}: {}", configFileName, e.what());
		}
	}
	void InitConfig() {
		ConfigExists() ? ReadConfig() : CreateConfig();

		configPreviousWriteTime = std::filesystem::last_write_time(configFileName);
		configLastWriteTime = configPreviousWriteTime;
	}
	void ConfigLoop() {
		while (!Core::exiting) {
			Sleep(2000);

			if (!ConfigExists()) {
				CreateConfig();
				Sleep(750);
				configPreviousWriteTime = std::filesystem::last_write_time(configFileName);
				continue;
			}

			// Check for config changes
			configLastWriteTime = std::filesystem::last_write_time(configFileName);
			if (configLastWriteTime != configPreviousWriteTime && !savedConfig) {
				configPreviousWriteTime = configLastWriteTime;
				ReadConfig(true);
				continue;
			} else if (configLastWriteTime != configPreviousWriteTime && savedConfig) {
				configPreviousWriteTime = configLastWriteTime;
				savedConfig = false;
			}

			if (CheckForChangesInMem())
				SaveConfig();
		}
	}
}