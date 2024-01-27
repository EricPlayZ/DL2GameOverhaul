#include <Hotkey.h>
#include <imgui.h>
#include "camera.h"
#include "menu.h"
#include "player.h"
#include "world.h"

namespace Menu {
	static const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    static const ImVec2 minWndSize = ImVec2(0.0f, 0.0f);
    static const ImVec2 maxWndSize = ImVec2(900.0f, 675.0f);

    KeyBindOption menuToggle = KeyBindOption(VK_F5);
    float transparency = 99.0f;

	void Render() {
        ImGuiStyle* style = &ImGui::GetStyle();
        style->Colors[ImGuiCol_WindowBg] = ImVec4(style->Colors[ImGuiCol_WindowBg].x, style->Colors[ImGuiCol_WindowBg].y, style->Colors[ImGuiCol_WindowBg].z, static_cast<float>(transparency) / 100.0f);

        ImGui::SetNextWindowSizeConstraints(minWndSize, maxWndSize);
        ImGui::Begin("EGameTools", &menuToggle.value, windowFlags); {
            if (ImGui::BeginTabBar("##MainTabBar")) {
                if (ImGui::BeginTabItem("Player")) {
                    Player::Render();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Camera")) {
                    Camera::Render();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("World")) {
                    World::Render();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::Separator();

            ImGui::Hotkey("Menu Toggle Key", menuToggle);
            ImGui::SliderFloat("Menu Transparency", &transparency, 0.0f, 100.0f, "%.1f%%", ImGuiSliderFlags_AlwaysClamp);
            ImGui::End();
        }
	}
}