#include <filesystem>
#include <fstream>
#include <algorithm>
#include <spdlog\spdlog.h>
#include <ImGui\imgui_hotkey.h>
#include <ImGui\imguiex.h>
#include <ImGuiFileDialog\ImGuiFileDialog.h>
#include <EGSDK\Utils\Files.h>
#include <EGSDK\Engine\CBulletPhysicsCharacter.h>
#include <EGSDK\GamePH\FreeCamera.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\GamePH\PlayerHealthModule.h>
#include <EGSDK\GamePH\PlayerInfectionModule.h>
#include <EGSDK\GamePH\PlayerVariables.h>
#include <EGSDK\GamePH\GamePH_Misc.h>
#include <EGT\FileEmbeds\player_variables.scr.embed>
#include <EGT\Config\Config.h>
#include <EGT\Menu\Camera.h>
#include <EGT\Menu\Menu.h>
#include <EGT\Menu\Player.h>

namespace EGT::Menu {
	namespace Player {
		float playerHealth = 80.0f;
		float playerMaxHealth = 80.0f;
		float playerImmunity = 80.0f;
		float playerMaxImmunity = 80.0f;
		int oldWorldMoney = 0;
		ImGui::KeyBindOption godMode{ VK_F6 };
		ImGui::KeyBindOption freezePlayer{ VK_NONE };
		ImGui::KeyBindOption unlimitedImmunity{ VK_NONE };
		ImGui::KeyBindOption unlimitedStamina{ VK_NONE };
		ImGui::KeyBindOption unlimitedItems{ VK_NONE };
		ImGui::KeyBindOption oneHitKill{ VK_NONE };
		ImGui::KeyBindOption invisibleToEnemies{ VK_NONE };
		ImGui::KeyBindOption disableOutOfBoundsTimer{ VK_NONE };
		ImGui::KeyBindOption nightrunnerMode{ VK_F7 };
		ImGui::KeyBindOption oneHandedMode{ VK_NONE };
		ImGui::KeyBindOption allowGrappleHookInSafezone{ VK_NONE };
		ImGui::KeyBindOption disableAirControl{ VK_NONE };
		ImGui::Option playerVariables{ false };

		std::string saveSCRPath{};
		std::string loadSCRFilePath{};

		static bool debugEnabled = false;
		static bool restoreVarsToSavedVarsEnabled = false;
		static char playerVarsSearchFilter[64];

        static std::string getParamName(const std::string &str) {
			size_t firstQuotePos = str.find_first_of("\"");
			if (firstQuotePos == std::string::npos)
				return {};
			size_t lastQuotePos = str.find_first_of("\"", firstQuotePos + 1);
			if (lastQuotePos == std::string::npos)
				return {};

			return str.substr(firstQuotePos + 1, lastQuotePos - firstQuotePos - 1);
        }
		static std::string getParamValue(const std::string &str) {
			size_t firstQuotePos = str.find_first_of("\"");
			if (firstQuotePos == std::string::npos)
				return {};
			size_t secondQuotePos = str.find_first_of("\"", firstQuotePos + 1);
			if (secondQuotePos == std::string::npos)
				return {};
			size_t thirdQuotePos = str.find_first_of("\"", secondQuotePos + 1);
			if (thirdQuotePos == std::string::npos)
				return {};
			size_t lastQuotePos = str.find_first_of("\"", thirdQuotePos + 1);
			if (lastQuotePos == std::string::npos)
				return {};

			return str.substr(thirdQuotePos + 1, lastQuotePos - thirdQuotePos - 1);
		}
        static void replaceParamValue(std::string& str, const std::string& newValue) {
			const std::string value = getParamValue(str);
			if (value.empty())
				return;

			EGSDK::Utils::Values::str_replace(str, value, newValue);
        }

		static void PlayerPositionUpdate() {
			EGSDK::Engine::CBulletPhysicsCharacter* playerCharacter = EGSDK::Engine::CBulletPhysicsCharacter::Get();
			if (!playerCharacter)
				return;
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel)
				return;

			if ((freezePlayer.GetValue() || (Camera::freeCam.GetValue() && !Camera::teleportPlayerToCamera.GetValue())) && !iLevel->IsTimerFrozen()) {
				playerCharacter->FreezeCharacter();
				return;
			}

			EGSDK::Engine::CBulletPhysicsCharacter::posBeforeFreeze = playerCharacter->playerPos;

			if (iLevel->IsTimerFrozen() || !Camera::freeCam.GetValue() || !Camera::teleportPlayerToCamera.GetValue())
				return;

			EGSDK::GamePH::FreeCamera* freeCam = EGSDK::GamePH::FreeCamera::Get();
			if (!freeCam)
				return;

			EGSDK::Vector3 camPos{};
			freeCam->GetPosition(&camPos);
			if (camPos.isDefault())
				return;

			playerCharacter->MoveCharacter(camPos);
		}
		static void PlayerVarsUpdate() {
			if (!playerVariables.GetValue())
				return;

			for (const auto& customPlayerVar : EGSDK::GamePH::PlayerVariables::customPlayerVars) {
				if (EGSDK::GamePH::PlayerVariables::IsPlayerVarManagedByBool(customPlayerVar->GetName()))
					continue;

				switch (customPlayerVar->GetType()) {
				case EGSDK::GamePH::PlayerVarType::String:
					// TO IMPLEMENT
					break;
				case EGSDK::GamePH::PlayerVarType::Float:
				{
					EGSDK::GamePH::FloatPlayerVariable* customFloatPlayerVar = reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(customPlayerVar.get());
					EGSDK::GamePH::PlayerVariables::ChangePlayerVar(customFloatPlayerVar->GetName(), customFloatPlayerVar->value.data);
					break;
				}
				case EGSDK::GamePH::PlayerVarType::Bool:
				{
					EGSDK::GamePH::BoolPlayerVariable* customBoolPlayerVar = reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(customPlayerVar.get());
					EGSDK::GamePH::PlayerVariables::ChangePlayerVar(customBoolPlayerVar->GetName(), customBoolPlayerVar->value.data);
					break;
				}
				default:
					break;
				}
			}
		}
		static void PlayerHealthUpdate() {
			EGSDK::GamePH::PlayerHealthModule* playerHealthModule = EGSDK::GamePH::PlayerHealthModule::Get();
			if (!playerHealthModule)
				return;

			playerMaxHealth = playerHealthModule->maxHealth;
			
			if (menuToggle.GetValue())
				return;
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;

			playerHealth = playerHealthModule->health;
		}
		static void PlayerImmunityUpdate() {
			EGSDK::GamePH::PlayerInfectionModule* playerInfectionModule = EGSDK::GamePH::PlayerInfectionModule::Get();
			if (!playerInfectionModule)
				return;

			playerMaxImmunity = playerInfectionModule->maxImmunity * 100.0f;

			if (menuToggle.GetValue())
				return;
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;

			playerImmunity = playerInfectionModule->immunity * 100.0f;
		}
		static void UpdatePlayerVars() {
			if (!EGSDK::GamePH::PlayerVariables::gotPlayerVars)
				return;

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("NightRunnerItemForced", true, false, nightrunnerMode.GetValue());
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("NightRunnerFurySmashEnabled", true, false, nightrunnerMode.GetValue());
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("NightRunnerFuryGroundPoundEnabled", true, false, nightrunnerMode.GetValue());

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("AntizinDrainBlocked", true, false, unlimitedImmunity.GetValue());

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("InfiniteStamina", true, false, unlimitedStamina.GetValue());

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("UvFlashlightEnergyDrainFactor", 0.0f, 1.0f, unlimitedItems.GetValue());

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("DamageMulAll", 99999.0f, 0.0f, oneHitKill.GetValue(), true);

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("InVisibleToEnemies", true, false, invisibleToEnemies.GetValue());

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("LeftHandDisabled", true, false, oneHandedMode.GetValue());
		}
		static void UpdateDisabledOptions() {
			freezePlayer.SetChangesAreDisabled(!EGSDK::Engine::CBulletPhysicsCharacter::Get());
		}
		static void HandleToggles() {
			if (disableAirControl.HasChanged()) {
				disableAirControl.SetPrevValue(disableAirControl.GetValue());
				EGSDK::GamePH::ReloadJumps();
			}
		}

		static void SaveVariablesToSCR() {
			if (!std::filesystem::exists(saveSCRPath))
				return;

			std::string tempPlayerVarsSCR = g_PlayerVariablesSCRFile;

			std::istringstream iss(tempPlayerVarsSCR);
			std::string line{};
			while (std::getline(iss, line)) {
				const std::string origLine = line;
				const std::string name = getParamName(line);
				if (name.empty())
					continue;

				auto playerVar = EGSDK::GamePH::PlayerVariables::playerVars.Find(name);
				if (!playerVar)
					continue;

				switch (playerVar->GetType()) {
				case EGSDK::GamePH::PlayerVarType::String:
					// TO IMPLEMENT
					break;
				case EGSDK::GamePH::PlayerVarType::Float: {
					EGSDK::GamePH::FloatPlayerVariable* floatPlayerVar = reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(playerVar);
					replaceParamValue(line, std::to_string(floatPlayerVar->value));
					break;
				}
				case EGSDK::GamePH::PlayerVarType::Bool: {
					EGSDK::GamePH::BoolPlayerVariable* boolPlayerVar = reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(playerVar);
					replaceParamValue(line, boolPlayerVar ? "true" : "false");
					break;
				}
				default:
					break;
				}
				EGSDK::Utils::Values::str_replace(tempPlayerVarsSCR, origLine, line);
			}

			std::ofstream outFile(std::string(saveSCRPath) + "\\player_variables.scr", std::ios::binary);
			if (!outFile.is_open()) {
				ImGui::OpenPopup("Failed saving player variables.");
				return;
			}
			outFile << tempPlayerVarsSCR;
			outFile.close();

			ImGui::OpenPopup("Saved player variables!");
		}
		static void LoadPlayerVariablesSCR() {
			if (!std::filesystem::exists(loadSCRFilePath))
				return;

			std::ifstream file(loadSCRFilePath);
			if (!file.is_open()) {
				ImGui::OpenPopup("Failed loading player variables.");
				return;
			}

			std::string line{};
			while (std::getline(file, line)) {
				std::string name = getParamName(line);
				if (name.empty())
					continue;
				std::string value = getParamValue(line);
				if (value.empty())
					continue;

				EGSDK::GamePH::PlayerVariables::ChangePlayerVar(name, value);
			}
			file.close();

			ImGui::OpenPopup("Loaded player variables!");
		}

		static void RestoreVariablesToDefault() {
			for (auto const& playerVarPtr : EGSDK::GamePH::PlayerVariables::playerVars) {
				auto playerVar = playerVarPtr.get();

				auto& defVars = restoreVarsToSavedVarsEnabled ? EGSDK::GamePH::PlayerVariables::customDefaultPlayerVars : EGSDK::GamePH::PlayerVariables::defaultPlayerVars;
				auto defPlayerVar = defVars.Find(playerVar->GetName());
				if (!defPlayerVar)
					continue;

				switch (playerVar->GetType()) {
				case EGSDK::GamePH::PlayerVarType::String:
					// TO IMPLEMENT
					break;
				case EGSDK::GamePH::PlayerVarType::Float: {
					EGSDK::GamePH::FloatPlayerVariable* defFloatPlayerVar = reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(defPlayerVar);
					EGSDK::GamePH::PlayerVariables::ChangePlayerVar(playerVar->GetName(), defFloatPlayerVar->value.data);
					break;
				}
				case EGSDK::GamePH::PlayerVarType::Bool: {
					EGSDK::GamePH::BoolPlayerVariable* defBoolPlayerVar = reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(defPlayerVar);
					EGSDK::GamePH::PlayerVariables::ChangePlayerVar(playerVar->GetName(), defBoolPlayerVar->value.data);
					break;
				}
				default:
					break;
				}

				if (!restoreVarsToSavedVarsEnabled)
					defVars.Erase(playerVar->GetName());
			}

			ImGui::OpenPopup("Restored player variables!");
		}
		static void SaveVariablesAsDefault() {
			for (auto const& playerVarPtr : EGSDK::GamePH::PlayerVariables::playerVars) {
				auto playerVar = playerVarPtr.get();

				auto defCustomPlayerVar = EGSDK::GamePH::PlayerVariables::customDefaultPlayerVars.Find(playerVar->GetName());
				if (!defCustomPlayerVar)
					continue;

				switch (playerVar->GetType()) {
				case EGSDK::GamePH::PlayerVarType::String:
					// TO IMPLEMENT
					break;
				case EGSDK::GamePH::PlayerVarType::Float: {
					EGSDK::GamePH::FloatPlayerVariable* floatPlayerVar = reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(playerVar);
					EGSDK::GamePH::FloatPlayerVariable* defCustomFloatPlayerVar = reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(defCustomPlayerVar);
					defCustomFloatPlayerVar->value = floatPlayerVar->value;
					break;
				}
				case EGSDK::GamePH::PlayerVarType::Bool: {
					EGSDK::GamePH::BoolPlayerVariable* boolPlayerVar = reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(playerVar);
					EGSDK::GamePH::BoolPlayerVariable* defCustomBoolPlayerVar = reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(defCustomPlayerVar);
					defCustomBoolPlayerVar->value = boolPlayerVar->value;
					break;
				}
				default:
					break;
				}
			}

			ImGui::OpenPopup("Saved current player variables!");
		}
		static void RestoreVariableToDefault(const std::string& name) {
			auto& defVars = restoreVarsToSavedVarsEnabled ? EGSDK::GamePH::PlayerVariables::customDefaultPlayerVars : EGSDK::GamePH::PlayerVariables::defaultPlayerVars;
			auto defPlayerVar = defVars.Find(name);
			if (!defPlayerVar)
				return;

			switch (defPlayerVar->GetType()) {
			case EGSDK::GamePH::PlayerVarType::String:
				// TO IMPLEMENT
				break;
			case EGSDK::GamePH::PlayerVarType::Float: {
				EGSDK::GamePH::FloatPlayerVariable* defFloatPlayerVar = reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(defPlayerVar);
				EGSDK::GamePH::PlayerVariables::ChangePlayerVar(name, defFloatPlayerVar->value.data);
				break;
			}
			case EGSDK::GamePH::PlayerVarType::Bool: {
				EGSDK::GamePH::BoolPlayerVariable* defBoolPlayerVar = reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(defPlayerVar);
				EGSDK::GamePH::PlayerVariables::ChangePlayerVar(name, defBoolPlayerVar->value.data);
				break;
			}
			default:
				break;
			}

			if (!restoreVarsToSavedVarsEnabled) {
				defVars.Erase(name);
				auto customPlayerVar = EGSDK::GamePH::PlayerVariables::customPlayerVars.Find(name);
				if (!customPlayerVar)
					return;

				EGSDK::GamePH::PlayerVariables::customPlayerVars.Erase(name);
			}
		}

		static bool shouldDisplayVariable(const std::unique_ptr<EGSDK::GamePH::PlayerVariable>& playerVarPtr, const std::string& searchFilter) {
			if (playerVarPtr->GetType() == EGSDK::GamePH::PlayerVarType::String || playerVarPtr->GetType() == EGSDK::GamePH::PlayerVarType::NONE) // TO IMPLEMENT
				return false;
			if (searchFilter.empty())
				return true;

			// Convert searchFilter to lowercase
			std::string lowerFilter = searchFilter;
			std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);

			// Convert variable name to lowercase and check if it contains the filter
			std::string lowerKey = playerVarPtr->GetName();
			std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
			return lowerKey.find(lowerFilter) != std::string::npos;
		}
		static void renderDebugInfo(const std::unique_ptr<EGSDK::GamePH::PlayerVariable>& playerVarPtr) {
			const float maxInputTextWidth = ImGui::CalcTextSize("0x0000000000000000").x;
			static std::string labelID{};
			labelID = "##DebugAddrInputText" + std::string(playerVarPtr->GetName());
			DWORD64 finalVarValueAddr = reinterpret_cast<DWORD64>(playerVarPtr.get()) + 0x8;

			std::stringstream ss;
			if (finalVarValueAddr)
				ss << "0x" << std::uppercase << std::hex << finalVarValueAddr;
			else
				ss << "NULL";

			static std::string addrString{};
			addrString = ss.str();

			ImGui::SameLine();

			//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ((ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f));
			ImGui::SetNextItemWidth(maxInputTextWidth);
			ImGui::PushStyleColor(ImGuiCol_Text, finalVarValueAddr ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
			ImGui::InputText(labelID.c_str(), const_cast<char*>(addrString.c_str()), strlen(addrString.c_str()), ImGuiInputTextFlags_ReadOnly);
			ImGui::PopStyleColor();
		}
		static void renderPlayerVariable(const std::unique_ptr<EGSDK::GamePH::PlayerVariable>& playerVarPtr) {
			ImGui::BeginDisabled(EGSDK::GamePH::PlayerVariables::IsPlayerVarManagedByBool(playerVarPtr->GetName()));
			{
				switch (playerVarPtr->GetType()) {
				case EGSDK::GamePH::PlayerVarType::String:
					// TO IMPLEMENT
					break;
				case EGSDK::GamePH::PlayerVarType::Float: {
					EGSDK::GamePH::FloatPlayerVariable* floatPlayerVar = reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(playerVarPtr.get());
					float newValue = floatPlayerVar->value;
					if (ImGui::InputFloat(floatPlayerVar->GetName(), &newValue))
						EGSDK::GamePH::PlayerVariables::ChangePlayerVarFromList(floatPlayerVar->GetName(), newValue, playerVarPtr.get());
					break;
				}
				case EGSDK::GamePH::PlayerVarType::Bool: {
					EGSDK::GamePH::BoolPlayerVariable* boolPlayerVar = reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(playerVarPtr.get());
					bool newValue = boolPlayerVar->value;
					if (ImGui::Checkbox(boolPlayerVar->GetName(), &newValue))
						EGSDK::GamePH::PlayerVariables::ChangePlayerVarFromList(boolPlayerVar->GetName(), newValue, playerVarPtr.get());
					break;
				}
				default:
					break;
				}
				ImGui::EndDisabled();
			}

			ImGui::SameLine();
			static std::string restoreBtnName{};
			restoreBtnName = "Restore##" + std::string(playerVarPtr->GetName());
			
			ImGui::BeginDisabled(EGSDK::GamePH::PlayerVariables::customPlayerVars.none_of(playerVarPtr->GetName()) || EGSDK::GamePH::PlayerVariables::IsPlayerVarManagedByBool(playerVarPtr->GetName()));
			{
				if (ImGui::Button(restoreBtnName.c_str(), "Restores player variable to default"))
					RestoreVariableToDefault(playerVarPtr->GetName());
				ImGui::EndDisabled();
			}

			if (debugEnabled)
				renderDebugInfo(playerVarPtr);
		}
		static void HandlePlayerVariablesList() {
			if (!playerVariables.GetValue())
				return;

			ImGui::BeginDisabled(!EGSDK::GamePH::PlayerVariables::gotPlayerVars); {
				if (ImGui::CollapsingHeader("Player variables list", ImGuiTreeNodeFlags_None)) {
					ImGui::Indent();

					if (ImGui::Button("Save variables to file", "Saves current player variables to chosen file inside the file dialog"))
						ImGuiFileDialog::Instance()->OpenDialog("ChooseSCRPath", "Choose Folder", nullptr, { saveSCRPath.empty() ? "." : saveSCRPath });
					ImGui::SameLine();
					if (ImGui::Button("Load variables from file", "Loads player variables from chosen file inside the file dialog"))
						ImGuiFileDialog::Instance()->OpenDialog("ChooseSCRLoadPath", "Choose File", ".scr", { loadSCRFilePath.empty() ? "." : loadSCRFilePath });

					ImGui::Checkbox("Restore variables to saved variables", &restoreVarsToSavedVarsEnabled, "Sets whether or not \"Restore variables to default\" should restore variables to the ones saved by \"Save current variables as default\"");
					ImGui::Checkbox("Debug Mode", &debugEnabled, "Shows text boxes alongside player variables, which will show the address in memory of each variable");

					if (ImGui::Button("Restore variables to default"))
						RestoreVariablesToDefault();
					ImGui::SameLine();
					if (ImGui::Button("Save current variables as default", "Saves the current player variables as default for whenever you use \"Restore variables to default\""))
						SaveVariablesAsDefault();

					ImGui::Separator();
					ImGui::InputTextWithHint("##VarsSearch", "Search variables", playerVarsSearchFilter, 64);

					for (auto const& playerVarPtr : EGSDK::GamePH::PlayerVariables::playerVars) {
						if (!shouldDisplayVariable(playerVarPtr, playerVarsSearchFilter))
							continue;

						renderPlayerVariable(playerVarPtr);
					}

					ImGui::Unindent();
				}
				ImGui::EndDisabled();
			}
		}
		static void HandleDialogs() {
			if (ImGuiFileDialog::Instance()->Display("ChooseSCRPath", ImGuiWindowFlags_NoCollapse, ImVec2(600.0f, 400.0f))) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					const std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
					if (!filePath.empty()) {
						saveSCRPath = ImGuiFileDialog::Instance()->GetCurrentPath();
						SaveVariablesToSCR();
					}
				}
				ImGuiFileDialog::Instance()->Close();
			}
			if (ImGuiFileDialog::Instance()->Display("ChooseSCRLoadPath", ImGuiWindowFlags_NoCollapse, ImVec2(600.0f, 400.0f))) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					loadSCRFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
					LoadPlayerVariablesSCR();
				}
				ImGuiFileDialog::Instance()->Close();
			}
			if (ImGui::BeginPopupModal("Failed reloading player jump parameters.", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Could not find any \"jump_parameters.scr\" inside \"EGameTools\\UserModFiles\"! Please make sure a \"jump_parameters.scr\" file is present in the directory mentioned earlier.");
				if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopupModal("Reloaded player jump parameters!", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Player jump parameters have been reloaded! from \"EGameTools\\UserModFiles\"");
				if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopupModal("Failed saving player variables.", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("There was an error opening a handle to the file \"%s\\player_variables.scr\"! The file is most likely already open in another program. Please close it!", saveSCRPath.c_str());
				if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopupModal("Saved player variables!", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Player variables have been saved to \"%s\\player_variables.scr\"!", saveSCRPath.c_str());
				if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopupModal("Failed loading player variables.", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("There was an error opening the file \"%s\"! The file is most likely already open in another program. Please close it!", loadSCRFilePath.c_str());
				if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopupModal("Loaded player variables!", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Player variables have been loaded from \"%s\"!", loadSCRFilePath.c_str());
				if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopupModal("Restored player variables!", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("All player variables have been restored to default (values from game load)!");
				if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopupModal("Saved current player variables!", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("All current player variables have been stored as default! Now if you have \"Restore variables to saved variables\" enabled, restoring to default will restore player variables to the ones you have saved!");
				if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}

		static void UpdateMoney(bool updateSlider) {
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;
			EGSDK::GamePH::PlayerDI_PH* player = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!player)
				return;
			EGSDK::GamePH::InventoryContainerDI* invContainer = player->GetInventoryContainer();
			if (!invContainer)
				return;
			EGSDK::GamePH::InventoryMoney* invMoney = invContainer->GetInventoryMoney(0);
			if (!invMoney)
				return;

			updateSlider ? (oldWorldMoney = invMoney->oldWorldMoney) : (invMoney->oldWorldMoney = oldWorldMoney);
		}
		static bool isMoneyInteractionDisabled() {
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return true;
			EGSDK::GamePH::PlayerDI_PH* player = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!player)
				return true;
			EGSDK::GamePH::InventoryContainerDI* invContainer = player->GetInventoryContainer();
			if (!invContainer || !invContainer->GetInventoryMoney(0))
				return true;

			return false;
		}

		Tab Tab::instance{};
		void Tab::Update() {
			PlayerPositionUpdate();
			PlayerHealthUpdate();
			PlayerImmunityUpdate();
			UpdateDisabledOptions();
			UpdatePlayerVars();
			PlayerVarsUpdate();
			HandleToggles();
		}
		void Tab::Render() {
			ImGui::SeparatorText("Misc");
			EGSDK::GamePH::PlayerHealthModule* playerHealthModule = EGSDK::GamePH::PlayerHealthModule::Get();
			ImGui::BeginDisabled(!playerHealthModule); {
				if (ImGui::SliderFloat("Player Health", &playerHealth, 0.0f, playerMaxHealth, "%.2f") && playerHealthModule)
					playerHealthModule->health = playerHealth;
				else if (playerHealthModule)
					playerHealth = playerHealthModule->health;
				ImGui::EndDisabled();
			}
			EGSDK::GamePH::PlayerInfectionModule* playerInfectionModule = EGSDK::GamePH::PlayerInfectionModule::Get();
			ImGui::BeginDisabled(!playerInfectionModule); {
				if (ImGui::SliderFloat("Player Immunity", &playerImmunity, 0.0f, playerMaxImmunity, "%.2f") && playerInfectionModule)
					playerInfectionModule->immunity = playerImmunity / 100.0f;
				else if (playerInfectionModule)
					playerImmunity = playerInfectionModule->immunity * 100.0f;
				ImGui::EndDisabled();
			}
			ImGui::BeginDisabled(isMoneyInteractionDisabled()); {
				if (ImGui::DragInt("Old World Money", &oldWorldMoney, 2.0f, 0, 999999999))
					UpdateMoney(false);
				else
					UpdateMoney(true);
				ImGui::EndDisabled();
			}
			ImGui::CheckboxHotkey("God Mode", &godMode, "Makes the player invincible");
			ImGui::SameLine();
			ImGui::BeginDisabled(freezePlayer.GetChangesAreDisabled()); {
				ImGui::CheckboxHotkey("Freeze Player", &freezePlayer, "Freezes player position");
				ImGui::EndDisabled();
			}
			ImGui::CheckboxHotkey("Unlimited Immunity", &unlimitedImmunity, "Stops immunity from draining");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("Unlimited Stamina", &unlimitedStamina, "Stops stamina from draining");
			ImGui::CheckboxHotkey("Unlimited Items", &unlimitedItems, "Stops the game from lowering the amount of items such as consumables / throwables when using them, alongside other inventory items such as ammo, lockpicks and other items;\nWARNING: This will not stop the item from getting removed from your inventory if you drop the entire amount\nCurrently, if the amount of item is 1, it will still drop from your inventory unfortunately");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("One-Hit Kill", &oneHitKill, "Makes the player one-hit kill EVERYTHING and EVERYONE RAWRRR");
			ImGui::CheckboxHotkey("Invisible to Enemies", &invisibleToEnemies, "Makes the player invisible to the enemies");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("Disable Out of Bounds Timer", &disableOutOfBoundsTimer, "Disables the timer that runs when out of map bounds or mission bounds");
			ImGui::CheckboxHotkey("Nightrunner Mode", &nightrunnerMode, "Makes Aiden super-human/infected");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("One-handed Mode", &oneHandedMode, "Removes Aiden's left hand");
			ImGui::CheckboxHotkey("Allow Grapple Hook in Safezone", &allowGrappleHookInSafezone, "Allows player to use grapple hook while in a safezone");

			ImGui::SeparatorText("Player Jump Parameters");
			ImGui::CheckboxHotkey("Disable Air Control", &disableAirControl, "Disables the ability to change the player's direction of momentum while jumping (in-air)");
			if (ImGui::Button("Reload Jump Params", "Reloads jump_parameters.scr from any mod located inside EGameTools\\UserModFiles")) {
				if (EGSDK::Utils::Files::FileExistsInDir("jump_parameters.scr", "EGameTools\\UserModFiles")) {
					EGSDK::GamePH::ReloadJumps();
					ImGui::OpenPopup("Reloaded player jump parameters!");
				} else
					ImGui::OpenPopup("Failed reloading player jump parameters.");
			}

			ImGui::SeparatorText("Player Variables");
			ImGui::Checkbox("Enabled##PlayerVars", &playerVariables, "Shows the list of player variables");
			HandlePlayerVariablesList();

			HandleDialogs();
		}
	}
}