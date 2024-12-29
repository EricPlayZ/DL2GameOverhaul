#pragma once
#include <string>
#include <set>
#include <imgui_hotkey.h>
#include <EGT\Core\core.h>

namespace EGT::Menu {
    class MenuTab {
    public:
        MenuTab(std::string_view name, int tabIndex) : tabName(name), tabIndex(tabIndex) { GetInstances()->insert({ tabIndex, this}); };
        ~MenuTab() { GetInstances()->erase({ tabIndex, this }); }
        static std::set<std::pair<int, MenuTab*>>* GetInstances() { static std::set<std::pair<int, MenuTab*>> instances{}; return &instances; };

        virtual void Render() {};
        virtual void Update() {};

        std::string_view tabName{};
        int tabIndex{};
    };

    extern const std::string title;
    extern ImGuiStyle defStyle;
    extern ImTextureID EGTLogoTexture;

	extern ImGui::KeyBindOption menuToggle;
	extern float opacity;
	extern float scale;

    extern ImGui::Option firstTimeRunning;
    extern ImGui::Option hasSeenChangelog;

    extern int currentTabIndex;

	extern void Render();
}