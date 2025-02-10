#include <filesystem>
#include <fstream>
#include <algorithm>
#include <spdlog\spdlog.h>
#include <ImGui\imgui_hotkey.h>
#include <ImGui\imguiex.h>
#include <ImGuiFileDialog\ImGuiFileDialog.h>
#include <EGSDK\Utils\Files.h>
#include <EGSDK\Engine\CBulletPhysicsCharacter.h>
#include <EGSDK\GamePH\CoPlayerRestrictions.h>
#include <EGSDK\GamePH\FreeCamera.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\GamePH\PlayerHealthModule.h>
#include <EGSDK\GamePH\PlayerInfectionModule.h>
#include <EGSDK\GamePH\PlayerVariables.h>
#include <EGSDK\GamePH\GamePH_Misc.h>
#include <EGT\ImGui_impl\DeferredActions.h>
#include <EGT\FileEmbeds\player_variables.scr.embed>
#include <EGT\GamePH\GamePH_Hooks.h>
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
		ImGui::KeyBindOption godMode{ false, VK_F6 };
		ImGui::KeyBindOption freezePlayer{ false, VK_NONE };
		ImGui::KeyBindOption unlimitedImmunity{ false, VK_NONE };
		ImGui::KeyBindOption unlimitedStamina{ false, VK_NONE };
		ImGui::KeyBindOption unlimitedItems{ false, VK_NONE };
		ImGui::KeyBindOption oneHitKill{ false, VK_NONE, true, { 11200 } };
		ImGui::KeyBindOption invisibleToEnemies{ false, VK_NONE };
		ImGui::KeyBindOption disableOutOfBoundsTimer{ false, VK_NONE };
		ImGui::KeyBindOption nightrunnerMode{ false, VK_F7 };
		ImGui::KeyBindOption oneHandedMode{ false, VK_NONE };
		ImGui::KeyBindOption disableSafezoneRestrictions{ false, VK_NONE };
		ImGui::KeyBindOption disableAirControl{ false, VK_NONE };
		ImGui::Option playerVariables{ false };

		std::string saveSCRPath{};
		std::string loadSCRFilePath{};

		static bool debugEnabled = false;
		static bool restoreVarsToSavedVarsEnabled = false;
		static char playerVarsSearchFilter[64];

#pragma region Player Variables
		static std::string getParamName(const std::string& str) {
			size_t firstQuotePos = str.find_first_of("\"");
			if (firstQuotePos == std::string::npos)
				return {};
			size_t lastQuotePos = str.find_first_of("\"", firstQuotePos + 1);
			if (lastQuotePos == std::string::npos)
				return {};

			return str.substr(firstQuotePos + 1, lastQuotePos - firstQuotePos - 1);
		}
		static std::string getParamValue(const std::string& str) {
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

		static void PlayerVarListValuesUpdate() {
			if (!playerVariables.GetValue())
				return;

			EGSDK::GamePH::PlayerVariables::customPlayerVars.ForEach([](std::unique_ptr<EGSDK::GamePH::PlayerVariable>& customPlayerVarPtr) {
				if (EGSDK::GamePH::PlayerVariables::IsPlayerVarManagedByBool(customPlayerVarPtr->GetName()))
					return;

				switch (customPlayerVarPtr->GetType()) {
					case EGSDK::GamePH::PlayerVarType::String:
						break; // TO IMPLEMENT
					case EGSDK::GamePH::PlayerVarType::Float:
						EGSDK::GamePH::PlayerVariables::ChangePlayerVar(customPlayerVarPtr->GetName(), reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(customPlayerVarPtr.get())->value.data);
						break;
					case EGSDK::GamePH::PlayerVarType::Bool:
						EGSDK::GamePH::PlayerVariables::ChangePlayerVar(customPlayerVarPtr->GetName(), reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(customPlayerVarPtr.get())->value.data);
						break;
					default:
						break;
				}
			});
		}
		static void PlayerVarsUpdate() {
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

		static void SaveVariablesToSCR() {
			if (!std::filesystem::exists(saveSCRPath)) {
				ImGui::OpenPopup("Failed saving player variables.");
				return;
			}

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
						break; // TO IMPLEMENT
					case EGSDK::GamePH::PlayerVarType::Float:
						replaceParamValue(line, std::to_string(reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(playerVar)->value));
						break;
					case EGSDK::GamePH::PlayerVarType::Bool:
						replaceParamValue(line, reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(playerVar) ? "true" : "false");
						break;
					default:
						break;
				}
				EGSDK::Utils::Values::str_replace(tempPlayerVarsSCR, origLine, line);
			}

			std::ofstream outFile(saveSCRPath + "\\player_variables.scr", std::ios::binary);
			if (!outFile.is_open()) {
				ImGui::OpenPopup("Failed saving player variables.");
				return;
			}
			outFile << tempPlayerVarsSCR;
			outFile.close();
			ImGui::OpenPopup("Saved player variables!");
		}
		static void LoadPlayerVariablesSCR() {
			if (!std::filesystem::exists(loadSCRFilePath)) {
				ImGui::OpenPopup("Failed loading player variables.");
				return;
			}

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

		static void RestoreVariableToDefault(const std::string& name) {
			ImGui_impl::DeferredActions::Add([name]() {
				auto& defVars = restoreVarsToSavedVarsEnabled ? EGSDK::GamePH::PlayerVariables::customDefaultPlayerVars : EGSDK::GamePH::PlayerVariables::defaultPlayerVars;
				auto defPlayerVar = defVars.Find(name);
				if (!defPlayerVar)
					return;

				switch (defPlayerVar->GetType()) {
					case EGSDK::GamePH::PlayerVarType::String:
						break; // TO IMPLEMENT
					case EGSDK::GamePH::PlayerVarType::Float:
						EGSDK::GamePH::PlayerVariables::ChangePlayerVar(name, reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(defPlayerVar)->value.data);
						break;
					case EGSDK::GamePH::PlayerVarType::Bool:
						EGSDK::GamePH::PlayerVariables::ChangePlayerVar(name, reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(defPlayerVar)->value.data);
						break;
					default:
						break;
				}

				if (!restoreVarsToSavedVarsEnabled) {
					defVars.Erase(name);
					EGSDK::GamePH::PlayerVariables::customPlayerVars.Erase(name);
				}
			});
		}
		static void RestoreVariablesToDefault() {
			(restoreVarsToSavedVarsEnabled ? EGSDK::GamePH::PlayerVariables::playerVars : EGSDK::GamePH::PlayerVariables::customPlayerVars).ForEach([](std::unique_ptr<EGSDK::GamePH::PlayerVariable>& playerVarPtr) {
				RestoreVariableToDefault(playerVarPtr->GetName());
			});
				
			ImGui::OpenPopup("Restored player variables!");
		}
		static void SaveVariablesAsDefault() {
			EGSDK::GamePH::PlayerVariables::playerVars.ForEach([](std::unique_ptr<EGSDK::GamePH::PlayerVariable>& playerVarPtr) {
				auto playerVar = playerVarPtr.get();
				auto defCustomPlayerVar = EGSDK::GamePH::PlayerVariables::customDefaultPlayerVars.Find(playerVar->GetName());
				if (!defCustomPlayerVar)
					return;

				switch (playerVar->GetType()) {
					case EGSDK::GamePH::PlayerVarType::String:
						break; // TO IMPLEMENT
					case EGSDK::GamePH::PlayerVarType::Float:
						reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(defCustomPlayerVar)->value = reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(playerVar)->value;
						break;
					case EGSDK::GamePH::PlayerVarType::Bool:
						reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(defCustomPlayerVar)->value = reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(playerVar)->value;
						break;
					default:
						break;
				}
			});
			ImGui::OpenPopup("Saved current player variables!");
		}

		static bool ShouldDisplayVariable(const std::unique_ptr<EGSDK::GamePH::PlayerVariable>& playerVarPtr, const std::string& searchFilter) {
			if (playerVarPtr->GetType() == EGSDK::GamePH::PlayerVarType::String || playerVarPtr->GetType() == EGSDK::GamePH::PlayerVarType::NONE) // TO IMPLEMENT
				return false;
			if (searchFilter.empty())
				return true;

			// Convert searchFilter and variable name to lowercase
			std::string lowerFilter = EGSDK::Utils::Values::to_lower(searchFilter);
			std::string lowerKey = EGSDK::Utils::Values::to_lower(playerVarPtr->GetName());
			return lowerKey.find(lowerFilter) != std::string::npos;
		}
		static void RenderDebugInfo(const std::unique_ptr<EGSDK::GamePH::PlayerVariable>& playerVarPtr) {
			const float maxInputTextWidth = ImGui::CalcTextSize("0x0000000000000000").x;
			std::string labelID = "##DebugAddrInputText" + std::string(playerVarPtr->GetName());
			DWORD64 finalVarValueAddr = reinterpret_cast<DWORD64>(playerVarPtr.get()) + 0x8; // 0x8 is PlayerVariable->value

			std::stringstream ss;
			if (finalVarValueAddr)
				ss << "0x" << std::uppercase << std::hex << finalVarValueAddr;
			else
				ss << "NULL";

			std::string addrString = ss.str();

			ImGui::SameLine();
			ImGui::SetNextItemWidth(maxInputTextWidth);
			ImGui::PushStyleColor(ImGuiCol_Text, finalVarValueAddr ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
			ImGui::InputText(labelID.c_str(), const_cast<char*>(addrString.c_str()), strlen(addrString.c_str()), ImGuiInputTextFlags_ReadOnly);
			ImGui::PopStyleColor();
		}
		static void RenderPlayerVariable(const std::unique_ptr<EGSDK::GamePH::PlayerVariable>& playerVarPtr) {
			auto playerVar = playerVarPtr.get();

			ImGui::BeginDisabled(EGSDK::GamePH::PlayerVariables::IsPlayerVarManagedByBool(playerVarPtr->GetName()));
			switch (playerVarPtr->GetType()) {
				case EGSDK::GamePH::PlayerVarType::String:
					break; // TO IMPLEMENT
				case EGSDK::GamePH::PlayerVarType::Float:
				{
					float newValue = reinterpret_cast<EGSDK::GamePH::FloatPlayerVariable*>(playerVar)->value;
					if (ImGui::InputFloat(playerVar->GetName(), &newValue))
						EGSDK::GamePH::PlayerVariables::ChangePlayerVarFromList(playerVar->GetName(), newValue, playerVar);
					break;
				}
				case EGSDK::GamePH::PlayerVarType::Bool:
				{
					bool newValue = reinterpret_cast<EGSDK::GamePH::BoolPlayerVariable*>(playerVar)->value;
					if (ImGui::Checkbox(playerVar->GetName(), &newValue))
						EGSDK::GamePH::PlayerVariables::ChangePlayerVarFromList(playerVar->GetName(), newValue, playerVar);
					break;
				}
				default:
					break;
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			std::string restoreBtnName = "Restore##" + std::string(playerVarPtr->GetName());
			
			ImGui::BeginDisabled(EGSDK::GamePH::PlayerVariables::customPlayerVars.none_of(playerVarPtr->GetName()) || EGSDK::GamePH::PlayerVariables::IsPlayerVarManagedByBool(playerVarPtr->GetName()));
			if (ImGui::Button(restoreBtnName.c_str(), "Restores player variable to default"))
				RestoreVariableToDefault(playerVarPtr->GetName());
			ImGui::EndDisabled();

			if (debugEnabled)
				RenderDebugInfo(playerVarPtr);
		}
		static void HandlePlayerVariablesList() {
			if (!playerVariables.GetValue())
				return;

			ImGui::BeginDisabled(!EGSDK::GamePH::PlayerVariables::gotPlayerVars);
			if (ImGui::CollapsingHeader("Player Variables list", ImGuiTreeNodeFlags_None)) {
				ImGui::Indent();

				if (ImGui::Button("Save variables to file", "Saves current player variables to chosen file inside the file dialog"))
					ImGuiFileDialog::Instance()->OpenDialog("ChooseSCRPath", "Choose Folder", nullptr, { saveSCRPath.empty() ? "." : saveSCRPath });
				ImGui::SameLine();
				if (ImGui::Button("Load variables from file", "Loads player variables from chosen file inside the file dialog"))
					ImGuiFileDialog::Instance()->OpenDialog("ChooseSCRLoadPath", "Choose File", ".scr", { loadSCRFilePath.empty() ? "." : loadSCRFilePath });

				ImGui::Checkbox("Restore variables to saved variables", &restoreVarsToSavedVarsEnabled, "Sets whether or not \"Restore variables to default\" should restore variables to the ones saved by \"Save current variables as default\"");
				ImGui::Checkbox("Debug Mode", &debugEnabled, "Shows text boxes alongside player variables, which will show the address in memory of each variable");

				ImGui::BeginDisabled(EGSDK::GamePH::PlayerVariables::customPlayerVars.empty());
				if (ImGui::Button("Restore variables to default"))
					RestoreVariablesToDefault();
				ImGui::EndDisabled();
				ImGui::SameLine();
				if (ImGui::Button("Save current variables as default", "Saves the current player variables as default for whenever you use \"Restore variables to default\""))
					SaveVariablesAsDefault();

				ImGui::Separator();
				ImGui::InputTextWithHint("##PlayerVariablesSearch", "Search variables", playerVarsSearchFilter, 64);

				EGSDK::GamePH::PlayerVariables::playerVars.ForEach([](std::unique_ptr<EGSDK::GamePH::PlayerVariable>& playerVarPtr) {
					if (ShouldDisplayVariable(playerVarPtr, playerVarsSearchFilter))
						RenderPlayerVariable(playerVarPtr);
				});

				ImGui::Unindent();
			}
			ImGui::EndDisabled();
		}
		static void HandlePlayerVariablesDialogs() {
			if (ImGuiFileDialog::Instance()->Display("ChooseSCRPath", ImGuiWindowFlags_NoCollapse, ImVec2(600.0f, 400.0f))) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					saveSCRPath = ImGuiFileDialog::Instance()->GetCurrentPath();
					SaveVariablesToSCR();
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
			ImGui::DisplaySimplePopupMessage("Failed reloading player jump parameters.", "Could not find any \"jump_parameters.scr\" inside \"EGameTools\\UserModFiles\"! Please make sure a\"jump_parameters.scr\" file is present in the directory mentioned earlier.");
			ImGui::DisplaySimplePopupMessage("Reloaded player jump parameters!", "Player jump parameters have been reloaded! from \"EGameTools\\UserModFiles\"");

			ImGui::DisplaySimplePopupMessage("Failed saving player variables.", "There was an error opening a handle to the file \"%s\\player_variables.scr\"! The file is most likely already open inanother program. Please close it!", saveSCRPath.c_str());
			ImGui::DisplaySimplePopupMessage("Saved player variables!", "Player variables have been saved to \"%s\\player_variables.scr\"!", saveSCRPath.c_str());

			ImGui::DisplaySimplePopupMessage("Failed loading player variables.", "There was an error opening the file \"%s\"! The file is most likely already open in another program. Please closeit!", loadSCRFilePath.c_str());
			ImGui::DisplaySimplePopupMessage("Loaded player variables!", "Player variables have been loaded from \"%s\"!", loadSCRFilePath.c_str());

			ImGui::DisplaySimplePopupMessage("Restored player variables!", "All player variables have been restored to default values!");
			ImGui::DisplaySimplePopupMessage("Saved current player variables!", "All current player variables have been stored as default! Now if you have \"Restore variables to saved variables\" enabled, restoring to default will restore player variables to the ones you have saved!");
		}
#pragma endregion

		static void PlayerPositionUpdate() {
			auto playerCharacter = EGSDK::Engine::CBulletPhysicsCharacter::Get();
			if (!playerCharacter)
				return;
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel)
				return;

			if ((freezePlayer.GetValue() || (Camera::freeCam.GetValue() && !Camera::teleportPlayerToCamera.GetValue())) && !iLevel->IsTimerFrozen()) {
				playerCharacter->FreezeCharacter();
				return;
			}

			EGSDK::Engine::CBulletPhysicsCharacter::posBeforeFreeze = playerCharacter->playerPos;

			if (iLevel->IsTimerFrozen() || !Camera::freeCam.GetValue() || !Camera::teleportPlayerToCamera.GetValue())
				return;

			auto freeCam = EGSDK::GamePH::FreeCamera::Get();
			if (!freeCam)
				return;

			EGSDK::Vec3 camPos{};
			freeCam->GetPosition(&camPos);
			if (!camPos.isDefault())
				playerCharacter->MoveCharacter(camPos);
		}
		static void PlayerHealthUpdate() {
			auto playerHealthModule = EGSDK::GamePH::PlayerHealthModule::Get();
			if (!playerHealthModule)
				return;

			playerMaxHealth = playerHealthModule->maxHealth;

			if (menuToggle.GetValue())
				return;
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;

			playerHealth = playerHealthModule->health;
		}
		static void PlayerImmunityUpdate() {
			auto playerInfectionModule = EGSDK::GamePH::PlayerInfectionModule::Get();
			if (!playerInfectionModule)
				return;

			static float previousNightrunnerTimer = playerInfectionModule->nightrunnerTimer;
			if (nightrunnerMode.GetValue()) {
				if (nightrunnerMode.HasChanged()) {
					previousNightrunnerTimer = playerInfectionModule->nightrunnerTimer;
					nightrunnerMode.SetPrevValue(true);
				}
				playerInfectionModule->nightrunnerTimer = FLT_MAX;
			}
			else if (nightrunnerMode.HasChanged()) {
				playerInfectionModule->nightrunnerTimer = previousNightrunnerTimer;
				nightrunnerMode.SetPrevValue(false);
			}

			playerMaxImmunity = playerInfectionModule->maxImmunity * 100.0f;

			if (menuToggle.GetValue())
				return;
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;

			playerImmunity = playerInfectionModule->immunity * 100.0f;
		}
		static void PlayerRestrictionsUpdate() {
			if (!disableSafezoneRestrictions.HasChanged())
				return;
			auto playerDI_PH = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!playerDI_PH || !playerDI_PH->areRestrictionsEnabledByGame)
				return;
			auto coPlayerRestrictions = EGSDK::GamePH::CoPlayerRestrictions::Get();
			if (!coPlayerRestrictions)
				return;

			DWORD64 tempFlags = 0;
			DWORD64* flagsPtr = coPlayerRestrictions->GetPlayerRestrictionsFlags(&tempFlags);
			if (flagsPtr)
				coPlayerRestrictions->flags = *flagsPtr;

			disableSafezoneRestrictions.HasChangedTo(false) ? playerDI_PH->EnablePlayerRestrictions(&*coPlayerRestrictions->flags.getPointer()) : playerDI_PH->DisablePlayerRestrictions(&*coPlayerRestrictions->flags.getPointer());

			disableSafezoneRestrictions.SetPrevValue(disableSafezoneRestrictions.GetValue());
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
		static void UpdateMoney(bool updateSlider) {
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;
			auto playerDI = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!playerDI)
				return;
			auto invContainer = playerDI->GetInventoryContainer();
			if (!invContainer)
				return;
			auto invMoney = invContainer->GetInventoryMoney(0);
			if (!invMoney)
				return;

			updateSlider ? (oldWorldMoney = invMoney->oldWorldMoney) : (invMoney->oldWorldMoney = oldWorldMoney);
		}
		static bool isMoneyInteractionDisabled() {
			auto iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return true;
			auto player = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!player)
				return true;
			auto invContainer = player->GetInventoryContainer();
			if (!invContainer || !invContainer->GetInventoryMoney(0))
				return true;

			return false;
		}

		Tab Tab::instance{};
		void Tab::Init() {}
		void Tab::Update() {
			UpdateDisabledOptions();

			PlayerPositionUpdate();
			PlayerHealthUpdate();
			PlayerImmunityUpdate();
			PlayerRestrictionsUpdate();

			PlayerVarsUpdate();
			PlayerVarListValuesUpdate();

			HandleToggles();
		}
		void Tab::Render() {
			ImGui::SeparatorText("Misc");
			auto playerHealthModule = EGSDK::GamePH::PlayerHealthModule::Get();
			ImGui::BeginDisabled(!playerHealthModule);
			if (ImGui::SliderFloat("Player Health", &playerHealth, 0.0f, playerMaxHealth, "%.2f") && playerHealthModule)
				playerHealthModule->health = playerHealth;
			else if (playerHealthModule)
				playerHealth = playerHealthModule->health;
			ImGui::EndDisabled();

			auto playerInfectionModule = EGSDK::GamePH::PlayerInfectionModule::Get();
			ImGui::BeginDisabled(!playerInfectionModule);
			if (ImGui::SliderFloat("Player Immunity", &playerImmunity, 0.0f, playerMaxImmunity, "%.2f") && playerInfectionModule)
				playerInfectionModule->immunity = playerImmunity / 100.0f;
			else if (playerInfectionModule)
				playerImmunity = playerInfectionModule->immunity * 100.0f;
			ImGui::EndDisabled();

			ImGui::BeginDisabled(isMoneyInteractionDisabled());
			UpdateMoney(!ImGui::DragInt("Old World Money", &oldWorldMoney, 2.0f, 0, 999999999));
			ImGui::EndDisabled();

			ImGui::CheckboxHotkey("God Mode", &godMode, "Makes the player invincible");
			ImGui::SameLine();
			ImGui::BeginDisabled(freezePlayer.GetChangesAreDisabled());
			ImGui::CheckboxHotkey("Freeze Player", &freezePlayer, "Freezes player position");
			ImGui::EndDisabled();

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
			ImGui::CheckboxHotkey("Disable Safezone Restrictions", &disableSafezoneRestrictions, "Disables all player restrictions inside a safezone, such as no jumping, climbing, weapons, etc; credits to @Synsteric on Discord for finding how to add this feature!");

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
			HandlePlayerVariablesDialogs();
		}
	}
}