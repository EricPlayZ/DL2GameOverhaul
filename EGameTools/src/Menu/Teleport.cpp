#include <spdlog\spdlog.h>
#include <ImGui\imgui_hotkey.h>
#include <ImGui\imguiex.h>
#include <EGSDK\Engine\CBulletPhysicsCharacter.h>
#include <EGSDK\GamePH\FreeCamera.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGT\Menu\Camera.h>
#include <EGT\Menu\Player.h>
#include <EGT\Menu\Teleport.h>

namespace EGT::Menu {
	namespace Teleport {
		std::string savedTeleportLocationsStr = "Bazaar - Highest Point:1944,123.6,932.8;Bazaar - Main Entrance:1962.9,50.1,927.9;Colonel Williams Stronghold - Main Entrance Bridge:994.3,22.8,-1138.6;Dynamo Cars Factory - Main Entrance:2295.9,-2.1,-78.6;Fish Eye - Player Safehouse:1180.4,32.4,-146.8;Fish Eye - Top of The Baloon:1122.6,98.8,-101.2;Observatory - Meeting Room:1951.2,-13.4,-329.6;Observatory - The 2 Domes (No Chemicals):1985.4,19.9,-357.2;Out of Bounds - Cut Road Quest:2693.3,-4.7,-241.5;PK Metro Station - Main Entrance:1886.9,50,628.9;PK Ship - Main Entrance:801.8,4.2,139.8;St. Paul Cathedral - GRE Entrance:463.4,4.2,-421;Tolga & Fatin Quest - Underground Loot Box:2343.9,12.2,-661.5;VNC Tower - \"V\" Logo:1434.2,4.3,-319.3;VNC Tower - Highest Player Safehouse:1424.7,354.6,-455;VNC Tower - Highest Point:1403.8,446.7,-389.8;X13 - Tunnel Near the City Walls Towards Facility:2407.9,36.2,-461.7;X13 - Underground Facility:2437.8,12.2,-649.9;X13 - Waltz Arena:2551.9,15.3,-569.1";
		std::vector<TeleportLocation> savedTeleportLocations{};
		static std::vector<std::string> savedTeleportLocationNames{};
		static std::vector<const char*> savedTeleportLocationNamesPtrs{};
		static int selectedTPLocation = -1;
		static char newLocationName[125]{};

		EGSDK::Vector3 waypointCoords{};
		bool* waypointIsSet = nullptr;
		bool justTeleportedToWaypoint = false;
		static EGSDK::Vector3 teleportCoords{};

		ImGui::KeyBindOption teleportToSelectedLocation{ VK_F9 };
		ImGui::KeyBindOption teleportToCoords{ VK_NONE };
		ImGui::KeyBindOption teleportToWaypoint{ VK_F10 };

		void UpdateTeleportLocationVisualNames() {
			savedTeleportLocationNames.clear();
			savedTeleportLocationNamesPtrs.clear();
			for (const auto& loc : savedTeleportLocations) {
				std::string completeName = loc.name + " (X: " + std::format("{:.1f}", loc.pos.X) + ", Y: " + std::format("{:.1f}", loc.pos.Y) + ", Z: " + std::format("{:.1f}", loc.pos.Z) + ")";
				savedTeleportLocationNames.emplace_back(completeName);
			}
			for (const auto& name : savedTeleportLocationNames)
				savedTeleportLocationNamesPtrs.emplace_back(name.c_str());
		}
		std::vector<TeleportLocation> ParseTeleportLocations(const std::string& input) {
			if (input.empty())
				return {};

			std::vector<TeleportLocation> teleportLocations{};
			std::stringstream ss(input);
			std::string item{};

			while (std::getline(ss, item, ';')) {
				size_t colonPos = item.rfind(':');
				if (colonPos != std::string::npos) {
					std::string tpLocName = item.substr(0, colonPos);
					std::string tpLocCoords = item.substr(colonPos + 1);

					std::stringstream coordStream(tpLocCoords);
					std::string coordItem{};
					std::vector<float> coordValues{};
					while (std::getline(coordStream, coordItem, ',')) {
						try {
							coordValues.push_back(std::stof(coordItem));
						} catch (...) {
							SPDLOG_ERROR("Invalid coordinate value: {}, for location: {}", coordItem, tpLocName);
							break;
						}
					}

					if (coordValues.size() == 3)
						teleportLocations.push_back({ tpLocName, { coordValues[0], coordValues[1], coordValues[2] } });
					else
						SPDLOG_ERROR("Invalid number of coordinates ({}) for location: {}", coordValues.size(), tpLocName);
				} else
					SPDLOG_ERROR("Invalid format for TP location: {}", item);
			}

			SPDLOG_INFO("Successfully parsed teleport locations:");
			int tpLocIndex = 1;
			for (const auto& loc : teleportLocations) {
				SPDLOG_INFO("{}. \"{}\" (X: {}, Y: {}, Z: {})", tpLocIndex, loc.name, loc.pos.X, loc.pos.Y, loc.pos.Z);
				tpLocIndex++;
			}

			return teleportLocations;
		}
		std::string ConvertTeleportLocationsToStr(const std::vector<TeleportLocation>& teleportLocations) {
			std::stringstream ss{};

			for (size_t i = 0; i < teleportLocations.size(); ++i) {
				const TeleportLocation& loc = teleportLocations[i];
				ss << loc.name << ":" << loc.pos.X << "," << loc.pos.Y << "," << loc.pos.Z;
				if (i < teleportLocations.size() - 1)
					ss << ";";
			}

			return ss.str();
		}
		static std::string GetFormattedPosition(const EGSDK::Vector3* position) {
			if (!position || position->isDefault())
				return "X: 0.00, Y: 0.00, Z: 0.00";
			static std::string formattedStr{};
			formattedStr = std::format("X: {:.2f}, Y: {:.2f}, Z: {:.2f}", position->X, position->Y, position->Z);
			return formattedStr;
		}

		static bool isTeleportationDisabled() {
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return true;
			if (!Camera::freeCam.GetValue() && !EGSDK::Engine::CBulletPhysicsCharacter::Get())
				return true;
			else if (Camera::freeCam.GetValue() && !EGSDK::GamePH::FreeCamera::Get())
				return true;

			return false;
		}

		static void SyncTPCoordsToPlayer() {
			if (isTeleportationDisabled())
				return;

			if (Camera::freeCam.GetValue()) {
				auto freeCam = EGSDK::GamePH::FreeCamera::Get();
				if (freeCam)
					freeCam->GetPosition(&teleportCoords);
			} else {
				auto playerCharacter = EGSDK::Engine::CBulletPhysicsCharacter::Get();
				if (playerCharacter)
					teleportCoords = playerCharacter->playerPos;
			}
		}
		static bool TeleportPlayerTo(const EGSDK::Vector3& pos) {
			if (isTeleportationDisabled() || pos.isDefault())
				return false;

			if (Camera::freeCam.GetValue()) {
				auto freeCam = EGSDK::GamePH::FreeCamera::Get();
				if (!freeCam)
					return false;
				freeCam->SetPosition(&pos);
			} else {
				auto playerCharacter = EGSDK::Engine::CBulletPhysicsCharacter::Get();
				if (!playerCharacter)
					return false;
				if (Player::freezePlayer.GetValue())
					playerCharacter->posBeforeFreeze = pos;
				playerCharacter->MoveCharacter(pos);
			}
			
			return true;
		}

		static void UpdateTeleportPos() {
			if (isTeleportationDisabled()) {
				if (!teleportCoords.isDefault())
					teleportCoords = {};
				return;
			}
			if (!teleportCoords.isDefault())
				return;

			if (Camera::freeCam.GetValue()) {
				auto freeCam = EGSDK::GamePH::FreeCamera::Get();
				if (freeCam)
					freeCam->GetPosition(&teleportCoords);
			} else {
				auto playerCharacter = EGSDK::Engine::CBulletPhysicsCharacter::Get();
				if (playerCharacter)
					teleportCoords = playerCharacter->playerPos;
			}
		}
		static void HotkeysUpdate() {
			teleportToSelectedLocation.SetChangesAreDisabled(selectedTPLocation < 0 || selectedTPLocation >= savedTeleportLocations.size());
			teleportToWaypoint.SetChangesAreDisabled(isTeleportationDisabled() || !waypointIsSet || !*waypointIsSet);
			teleportToCoords.SetChangesAreDisabled(isTeleportationDisabled());

			if (teleportToSelectedLocation.HasChanged()) {
				TeleportPlayerTo(savedTeleportLocations[selectedTPLocation].pos);
				teleportToSelectedLocation.SetPrevValue(teleportToSelectedLocation.GetValue());
			}
			if (teleportToWaypoint.HasChanged()) {
				justTeleportedToWaypoint = TeleportPlayerTo(waypointCoords);
				teleportToWaypoint.SetPrevValue(teleportToWaypoint.GetValue());
			}
			if (teleportToCoords.HasChanged()) {
				TeleportPlayerTo(teleportCoords);
				teleportToCoords.SetPrevValue(teleportToCoords.GetValue());
			}
		}

		static bool SaveTeleportLocation(const char* locationName) {
			if (isTeleportationDisabled()) {
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				ImGui::OpenPopup("Couldn't add location");
				return false;
			}

			auto savedLocIt = std::find_if(savedTeleportLocations.begin(), savedTeleportLocations.end(), [&locationName](const auto& loc) {
				return loc.name == locationName;
			});
			if (savedLocIt != savedTeleportLocations.end()) {
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				ImGui::OpenPopup("Location already exists");
				return false;
			}

			EGSDK::Vector3 playerPos{};

			if (Camera::freeCam.GetValue()) {
				EGSDK::GamePH::FreeCamera* freeCam = EGSDK::GamePH::FreeCamera::Get();
				if (!freeCam) {
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::OpenPopup("Couldn't add location");
					return false;
				}

				EGSDK::Vector3 camPos{};
				freeCam->GetPosition(&camPos);
				if (camPos.isDefault()) {
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::OpenPopup("Couldn't add location");
					return false;
				}

				playerPos = camPos;
			} else {
				EGSDK::Engine::CBulletPhysicsCharacter* playerCharacter = EGSDK::Engine::CBulletPhysicsCharacter::Get();
				if (!playerCharacter) {
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::OpenPopup("Couldn't add location");
					return false;
				}

				playerPos = playerCharacter->playerPos;
			}
			playerPos = playerPos.round(1);

			savedLocIt = std::find_if(savedTeleportLocations.begin(), savedTeleportLocations.end(), [&playerPos](const auto& loc) {
				return loc.pos == playerPos;
			});
			if (savedLocIt != savedTeleportLocations.end() && savedLocIt->pos == playerPos) {
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				ImGui::OpenPopup("Location already exists");
				return false;
			}

			savedTeleportLocations.emplace_back(std::string(locationName), playerPos);
			UpdateTeleportLocationVisualNames();

			ImGui::CloseCurrentPopup();
			return true;
		}
		static void HandleDialogs() {
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Give the location a name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				bool tpSaveResult = true;
				ImGui::PushItemWidth(510.0f * Menu::scale);
				if (ImGui::InputTextWithHint("##TPLocationNameInputText", "Location name", newLocationName, IM_ARRAYSIZE(newLocationName), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("OK", ImVec2(250.0f, 0.0f) * Menu::scale) && newLocationName[0]) {
					ImGui::PopItemWidth();
					tpSaveResult = SaveTeleportLocation(newLocationName);
					newLocationName[0] = 0;
				} else if (ImGui::SameLine(); ImGui::Button("Cancel", ImVec2(250.0f, 0.0f) * Menu::scale))
					ImGui::CloseCurrentPopup();
				if (tpSaveResult)
					ImGui::EndPopup();
			}
			ImGui::DisplaySimplePopupMessageCentered(500.0f, Menu::scale, "Location already exists", "The location you have entered already exists. Either the name of the location, or the position of the location is already inside the list. If you want to change it then please remove it and add it again.", false);
			ImGui::DisplaySimplePopupMessageCentered(500.0f, Menu::scale, "Couldn't add location", "Something went wrong trying to add a location. Either the player class or camera class are not found, or you're in a place in the game where the character or camera isn't properly loaded. If this happens, even though you know it should work fine, please contact @EricPlayZ on NexusMods, GitHub or Discord.");
		}

		Tab Tab::instance{};
		void Tab::Update() {
			UpdateTeleportPos();
			HotkeysUpdate();
		}
		void Tab::Render() {
			ImGui::SeparatorText("Saved Locations##Teleport");
			ImGui::SetNextItemWidth(672.0f * Menu::scale);
			ImGui::ListBox("##SavedTPLocationsListBox", &selectedTPLocation, savedTeleportLocationNamesPtrs.data(), static_cast<int>(savedTeleportLocationNamesPtrs.size()), 5);

			ImGui::BeginDisabled(isTeleportationDisabled() || selectedTPLocation < 0 || selectedTPLocation >= savedTeleportLocations.size());
			if (ImGui::ButtonHotkey("Teleport to Selected Location", &teleportToSelectedLocation, "Teleports player to selected location from the saved locations list"))
				TeleportPlayerTo(savedTeleportLocations[selectedTPLocation].pos);
			ImGui::EndDisabled();

			ImGui::SameLine();
			ImGui::BeginDisabled(selectedTPLocation < 0 || selectedTPLocation >= savedTeleportLocations.size());
			if (ImGui::Button("Remove Selected Location")) {
				savedTeleportLocations.erase(savedTeleportLocations.begin() + selectedTPLocation);
				UpdateTeleportLocationVisualNames();
				selectedTPLocation = -1;
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled(isTeleportationDisabled());
			ImGui::SameLine();
			if (ImGui::Button("Save Current Location"))
				ImGui::OpenPopup("Give the location a name");

			ImGui::EndDisabled();

			ImGui::SeparatorText("Custom##Teleport");
			ImGui::BeginDisabled(isTeleportationDisabled());
			auto playerCharacter = EGSDK::Engine::CBulletPhysicsCharacter::Get();
			auto freeCam = EGSDK::GamePH::FreeCamera::Get();

			EGSDK::Vector3 camPos{};
			ImGui::Text("Player Position: %s", GetFormattedPosition(playerCharacter ? &playerCharacter->playerPos.data : nullptr).data());
			ImGui::Text("Free Camera Position: %s", GetFormattedPosition(freeCam && Camera::freeCam.GetValue() ? freeCam->GetPosition(&camPos) : nullptr).data());
			ImGui::Text("Waypoint Position: %s", GetFormattedPosition(waypointIsSet && *waypointIsSet && !waypointCoords.isDefault() ? &waypointCoords : nullptr).data());

			ImGui::SetNextItemWidth(500.0f * Menu::scale);
			ImGui::InputFloat3("Teleport Coords (XYZ)", reinterpret_cast<float*>(&teleportCoords), "%.2fm");
			ImGui::EndDisabled();

			ImGui::BeginDisabled(isTeleportationDisabled() || !waypointIsSet || !*waypointIsSet);
			if (ImGui::ButtonHotkey("Teleport to Waypoint", &teleportToWaypoint, "Teleports player to waypoint.\nWARNING: If the waypoint is selected to track an object/item on the map, Teleport to Waypoint will not work, if so just set the waypoint nearby instead.\nWARNING: Your player height won't change when teleporting, so make sure you catch yourself if you fall under the map because of the teleportation"))
				justTeleportedToWaypoint = TeleportPlayerTo(waypointCoords);
			ImGui::EndDisabled();

			ImGui::BeginDisabled(isTeleportationDisabled());
			ImGui::SameLine();
			if (ImGui::ButtonHotkey("Teleport to Coords", &teleportToCoords, "Teleports player to the coords specified in the input boxes above"))
				TeleportPlayerTo(teleportCoords);
			ImGui::SameLine();
			if (ImGui::Button("Get Player Coords"))
				SyncTPCoordsToPlayer();
			ImGui::EndDisabled();

			HandleDialogs();
		}
	}
}