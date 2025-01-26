#pragma once
#include <vector>
#include <EGT\Menu\Menu.h>
#include <EGSDK\Vec3.h>
	
namespace EGT::Menu {
	namespace Teleport {
		struct TeleportLocation {
			std::string name{};
			EGSDK::Vec3 pos{};
		};

		extern std::string savedTeleportLocationsStr;
		extern std::vector<TeleportLocation> savedTeleportLocations;

		extern EGSDK::Vec3 waypointCoords;
		extern bool* waypointIsSet;
		extern bool justTeleportedToWaypoint;

		extern ImGui::KeyBindOption teleportToSelectedLocation;
		extern ImGui::KeyBindOption teleportToCoords;
		extern ImGui::KeyBindOption teleportToWaypoint;

		extern void UpdateTeleportLocationVisualNames();
		extern std::vector<TeleportLocation> ParseTeleportLocations(const std::string& input);
		extern std::string ConvertTeleportLocationsToStr(const std::vector<TeleportLocation>& teleportLocations);

		class Tab : MenuTab {
		public:
			Tab() : MenuTab("Teleport", 3) {}
			void Init() override;
			void Update() override;
			void Render() override;

			static Tab instance;
		};
	}
}