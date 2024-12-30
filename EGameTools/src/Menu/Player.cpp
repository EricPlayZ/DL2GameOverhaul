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
		ImGui::Option playerVariables{};

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

			auto bgn = EGSDK::GamePH::PlayerVariables::playerVars.begin();
			for (auto it = bgn; it != EGSDK::GamePH::PlayerVariables::playerVars.end(); ++it) {
				if (!it->second.first)
					continue;

				try {
					auto& valDef = EGSDK::GamePH::PlayerVariables::playerVarsDefault.at(it - bgn);

					if (it->second.second == "float") {
						float* varAddr = reinterpret_cast<float*>(it->second.first);
						if (!EGSDK::Utils::Values::are_samef(*varAddr, *(varAddr + 1)) && !EGSDK::Utils::Values::are_samef(*(varAddr + 1), std::any_cast<float>(valDef.second.first)))
							*varAddr = *(varAddr + 1);
					} else if (it->second.second == "bool") {
						bool* varAddr = reinterpret_cast<bool*>(it->second.first);
						if (*varAddr != *(varAddr + 1) && *(varAddr + 1) != std::any_cast<bool>(valDef.second.first))
							*varAddr = *(varAddr + 1);
					}
				} catch (std::exception& e) {
					UNREFERENCED_PARAMETER(e);
					SPDLOG_ERROR("PlayerVarsUpdate() threw an exception! Restart the game to fix this error. If the error still happens, please open a bug report.");
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
				const std::string paramName = getParamName(line);
				if (paramName.empty())
					continue;

				auto it = std::find_if(EGSDK::GamePH::PlayerVariables::playerVars.begin(), EGSDK::GamePH::PlayerVariables::playerVars.end(), [&paramName](const auto& pair) {
					return pair.first == paramName;
				});
				if (it == EGSDK::GamePH::PlayerVariables::playerVars.end())
					continue;

				if (it->second.second == "float") {
					float value = *reinterpret_cast<float*>(it->second.first);
					replaceParamValue(line, std::to_string(value));

				} else {
					bool value = *reinterpret_cast<bool*>(it->second.first);
					replaceParamValue(line, value ? "true" : "false");
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
				const std::string paramName = getParamName(line);
				if (paramName.empty())
					continue;
				const std::string paramValue = getParamValue(line);
				if (paramValue.empty())
					continue;

				EGSDK::GamePH::PlayerVariables::ChangePlayerVar(paramName, paramValue);
			}
			file.close();

			ImGui::OpenPopup("Loaded player variables!");
		}

		static void RestoreVariablesToDefault() {
			for (auto const& [key, val] : EGSDK::GamePH::PlayerVariables::playerVars) {
				if (!val.first)
					continue;

				auto& defVars = restoreVarsToSavedVarsEnabled ? EGSDK::GamePH::PlayerVariables::playerCustomVarsDefault : EGSDK::GamePH::PlayerVariables::playerVarsDefault;
				auto itDef = std::find_if(defVars.begin(), defVars.end(), [&key](const auto& pair) {
					return pair.first == key;
				});
				if (itDef == defVars.end())
					continue;

				if (val.second == "float")
					EGSDK::GamePH::PlayerVariables::ChangePlayerVar(key, std::any_cast<float>(itDef->second.first));
				else
					EGSDK::GamePH::PlayerVariables::ChangePlayerVar(key, std::any_cast<bool>(itDef->second.first));
			}

			ImGui::OpenPopup("Restored player variables!");
		}
		static void SaveVariablesAsDefault() {
			for (auto const& [key, val] : EGSDK::GamePH::PlayerVariables::playerVars) {
				if (!val.first)
					continue;

				auto itCustomDef = std::find_if(EGSDK::GamePH::PlayerVariables::playerCustomVarsDefault.begin(), EGSDK::GamePH::PlayerVariables::playerCustomVarsDefault.end(), [&key](const auto& pair) {
					return pair.first == key;
				});
				if (itCustomDef == EGSDK::GamePH::PlayerVariables::playerCustomVarsDefault.end())
					continue;

				if (val.second == "float") {
					float* const varAddr = reinterpret_cast<float*>(val.first);
					itCustomDef->second.first = *varAddr;
				} else if (val.second == "bool") {
					bool* const varAddr = reinterpret_cast<bool*>(val.first);
					itCustomDef->second.first = *varAddr;
				}
			}

			ImGui::OpenPopup("Saved current player variables!");
		}
		static void RestoreVariableToDefault(const std::string& varName) {
			auto& defVars = restoreVarsToSavedVarsEnabled ? EGSDK::GamePH::PlayerVariables::playerCustomVarsDefault : EGSDK::GamePH::PlayerVariables::playerVarsDefault;
			auto itDef = std::find_if(defVars.begin(), defVars.end(), [&varName](const auto& pair) {
				return pair.first == varName;
			});
			if (itDef == defVars.end())
				return;

			if (itDef->second.second == "float")
				EGSDK::GamePH::PlayerVariables::ChangePlayerVar(varName, std::any_cast<float>(itDef->second.first));
			else
				EGSDK::GamePH::PlayerVariables::ChangePlayerVar(varName, std::any_cast<bool>(itDef->second.first));
		}

		static bool shouldDisplayVariable(const std::string& key, const std::string& searchFilter) {
			if (searchFilter.empty()) return true;

			// Convert searchFilter to lowercase
			std::string lowerFilter = searchFilter;
			std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);

			// Convert key to lowercase and check if it contains the filter
			std::string lowerKey = key;
			std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
			return lowerKey.find(lowerFilter) != std::string::npos;
		}
		static void renderDebugInfo(const std::string& key, const std::pair<void*, std::string>& val) {
			const float maxInputTextWidth = ImGui::CalcTextSize("0x0000000000000000").x;
			static std::string labelID{};
			labelID = "##DebugAddrInputText" + std::string(key);
			DWORD64 finalAddr = val.second == "float" ? reinterpret_cast<DWORD64>(reinterpret_cast<float*>(val.first)) : reinterpret_cast<DWORD64>(reinterpret_cast<bool*>(val.first));

			std::stringstream ss;
			if (finalAddr)
				ss << "0x" << std::uppercase << std::hex << finalAddr;
			else
				ss << "NULL";

			static std::string addrString{};
			addrString = ss.str();

			ImGui::SameLine();

			//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ((ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2.0f));
			ImGui::SetNextItemWidth(maxInputTextWidth);
			ImGui::PushStyleColor(ImGuiCol_Text, finalAddr ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
			ImGui::InputText(labelID.c_str(), const_cast<char*>(addrString.c_str()), strlen(addrString.c_str()), ImGuiInputTextFlags_ReadOnly);
			ImGui::PopStyleColor();
		}
		static void renderPlayerVariable(const std::string& key, const std::pair<void*, std::string>& val) {
			float* floatVarAddr = nullptr;
			bool* boolVarAddr = nullptr;

			if (val.second == "float") {
				floatVarAddr = reinterpret_cast<float*>(val.first);
				float newValue = *floatVarAddr;
				if (ImGui::InputFloat(key.c_str(), &newValue)) {
					*floatVarAddr = newValue;
					*(floatVarAddr + 1) = newValue;
				}
			} else if (val.second == "bool") {
				boolVarAddr = reinterpret_cast<bool*>(val.first);
				bool newValue = *boolVarAddr;
				if (ImGui::Checkbox(key.c_str(), &newValue)) {
					*boolVarAddr = newValue;
					*(boolVarAddr + 1) = newValue;
				}
			}

			ImGui::SameLine();
			static std::string restoreBtnName{};
			restoreBtnName = "Restore##" + key;
			if (ImGui::Button(restoreBtnName.c_str(), "Restores player variable to default"))
				RestoreVariableToDefault(key);

			if (debugEnabled)
				renderDebugInfo(key, val);
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

					for (auto const& [key, val] : EGSDK::GamePH::PlayerVariables::playerVars) {
						if (!val.first || !shouldDisplayVariable(key, playerVarsSearchFilter))
							continue;

						renderPlayerVariable(key, val);
					}

					ImGui::Unindent();
				}
				ImGui::EndDisabled();
			}
		}
		void HandleDialogs() {
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
			PlayerVarsUpdate();
			PlayerHealthUpdate();
			PlayerImmunityUpdate();
			UpdateDisabledOptions();
			UpdatePlayerVars();
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