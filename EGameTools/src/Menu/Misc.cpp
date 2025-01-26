#include <ImGui\imgui_hotkey.h>
#include <ImGui\imguiex.h>
#include <EGSDK\Utils\Hook.h>
#include <EGSDK\Engine\RendererCVars.h>
#include <EGSDK\GamePH\GameDI_PH.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGT\ImGui_impl\DeferredActions.h>
#include <EGT\Menu\Misc.h>
#include <EGT\GamePH\GamePH_Hooks.h>

namespace EGT::Menu {
	namespace Misc {
		ImGui::KeyBindOption disableGamePauseWhileAFK{ VK_NONE };
		ImGui::KeyBindOption disableHUD{ VK_F8 };
		ImGui::Option disableSavegameCRCCheck{ false };
		ImGui::Option disableDataPAKsCRCCheck{ false };
		ImGui::Option increaseDataPAKsLimit{ false };

		static char rendererCVarsSearchFilter[64];

		static void UpdateDisabledOptions() {
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			disableHUD.SetChangesAreDisabled(!iLevel || !iLevel->IsLoaded());
		}

		static bool ShouldDisplayVariable(const std::unique_ptr<EGSDK::Engine::RendererCVar>& rendererCVarPtr, const std::string& searchFilter) {
			if (rendererCVarPtr->GetType() == EGSDK::Engine::RendererCVarType::NONE)
				return false;
			if (EGSDK::Utils::Values::are_samef(reinterpret_cast<EGSDK::Engine::FloatRendererCVar*>(rendererCVarPtr.get())->GetValue(), -404.0f))
				return false;
			if (searchFilter.empty())
				return true;

			// Convert searchFilter and variable name to lowercase
			std::string lowerFilter = EGSDK::Utils::Values::to_lower(searchFilter);
			std::string lowerKey = EGSDK::Utils::Values::to_lower(rendererCVarPtr->GetName());
			return lowerKey.find(lowerFilter) != std::string::npos;
		}
		static void RestoreVariableToDefault(const std::string& name) {
			ImGui_impl::DeferredActions::Add([name]() {
				auto defRendererCVar = EGSDK::Engine::RendererCVars::defaultRendererCVars.Find(name);
				if (!defRendererCVar)
					return;

				EGSDK::Engine::RendererCVars::ChangeRendererCVar(name, reinterpret_cast<EGSDK::Engine::FloatRendererCVar*>(defRendererCVar)->GetValue());

				EGSDK::Engine::RendererCVars::defaultRendererCVars.Erase(name);
				EGSDK::Engine::RendererCVars::customRendererCVars.Erase(name);
			});
		}
		static void RenderRendererCVar(const std::unique_ptr<EGSDK::Engine::RendererCVar>& rendererCVarPtr) {
			auto rendererCVar = rendererCVarPtr.get();

			float newValue = reinterpret_cast<EGSDK::Engine::FloatRendererCVar*>(rendererCVar)->GetValue();
			if (ImGui::InputFloat(rendererCVar->GetName(), &newValue))
				EGSDK::Engine::RendererCVars::ChangeRendererCVarFromList(rendererCVar->GetName(), newValue, rendererCVar);

			ImGui::SameLine();
			std::string restoreBtnName = "Restore##" + std::string(rendererCVarPtr->GetName());

			ImGui::BeginDisabled(EGSDK::Engine::RendererCVars::customRendererCVars.none_of(rendererCVarPtr->GetName()));
			if (ImGui::Button(restoreBtnName.c_str(), "Restores renderer cvar to default"))
				RestoreVariableToDefault(rendererCVarPtr->GetName());
			ImGui::EndDisabled();
		}
		static void HandleRendererCVarsList() {
			ImGui::BeginDisabled(EGSDK::Engine::RendererCVars::rendererCVars.empty());
			if (ImGui::CollapsingHeader("Renderer CVars list", ImGuiTreeNodeFlags_None)) {
				ImGui::Indent();

				ImGui::Separator();
				ImGui::InputTextWithHint("##RendererCVarsSearch", "Search variables", rendererCVarsSearchFilter, 64);

				EGSDK::Engine::RendererCVars::rendererCVars.ForEach([](std::unique_ptr<EGSDK::Engine::RendererCVar>& rendererCVarPtr) {
					if (ShouldDisplayVariable(rendererCVarPtr, rendererCVarsSearchFilter))
						RenderRendererCVar(rendererCVarPtr);
				});

				ImGui::Unindent();
			}
			ImGui::EndDisabled();
		}

		Tab Tab::instance{};
		void Tab::Update() {
			UpdateDisabledOptions();

			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel)
				return;

			if (!iLevel->IsLoaded() && disableHUD.GetValue()) {
				disableHUD.SetBothValues(false);
				iLevel->ShowUIManager(true);
				return;
			}
			if (disableHUD.HasChanged()) {
				disableHUD.SetPrevValue(disableHUD.GetValue());
				iLevel->ShowUIManager(!disableHUD.GetValue());
			}

			auto gameDI_PH = EGSDK::GamePH::GameDI_PH::Get();
			if (gameDI_PH)
				gameDI_PH->blockPauseGameOnPlayerAfk = disableGamePauseWhileAFK.GetValue();
		}
		void Tab::Render() {
			ImGui::SeparatorText("Misc##Misc");
			ImGui::CheckboxHotkey("Disable Game Pause While AFK", &disableGamePauseWhileAFK, "Prevents the game from pausing while you're afk");
			ImGui::SameLine();

			ImGui::BeginDisabled(disableHUD.GetChangesAreDisabled());
			ImGui::CheckboxHotkey("Disable HUD", &disableHUD, "Disables the entire HUD, including any sort of menus like the pause menu");
			ImGui::EndDisabled();

			ImGui::SeparatorText("Scripting##Misc");
			HandleRendererCVarsList();

			ImGui::SeparatorText("Game Checks##Misc");
			if (ImGui::Checkbox("Disable Savegame CRC Check *", &disableSavegameCRCCheck, "Stops the game from falsely saying your savegame is corrupt whenever you modify it outside of the game using a save editor"))
				disableSavegameCRCCheck.GetValue() ? EGT::GamePH::Hooks::SaveGameCRCBoolCheckHook.Enable() : EGT::GamePH::Hooks::SaveGameCRCBoolCheckHook.Disable();
			ImGui::SameLine();
			ImGui::Checkbox("Disable Data PAKs CRC Check *", &disableDataPAKsCRCCheck, "Stops the game from scanning data PAKs, which allows you to use data PAK mods in multiplayer as well");
			ImGui::Checkbox("Increase Data PAKs Limit *", &increaseDataPAKsLimit, "Allows you to add more than 8 data PAKs, e.g. data8.pak, data9.pak, data10.pak, etc, up to 200 PAKs in total");
			ImGui::Separator();
			ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(IM_COL32(200, 0, 0, 255)), "* Option requires game restart to apply");
		}
	}
}