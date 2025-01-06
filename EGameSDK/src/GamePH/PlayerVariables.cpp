#include <Windows.h>
#include <algorithm>
#include <spdlog\spdlog.h>
#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\PlayerState.h>
#include <EGSDK\GamePH\PlayerVariables.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static constexpr int STRING_SIZE_OFFSET = 3;
	static constexpr int FLOAT_SIZE_OFFSET = 3;
	static constexpr int BOOL_SIZE_OFFSET = 2;

	std::unordered_map<PlayerVariable*, std::string> PlayerVariable::playerVarNames{};
	std::unordered_map<PlayerVariable*, PlayerVarType> PlayerVariable::playerVarTypes{};
	PlayerVariable::PlayerVariable(const std::string& name) {
		playerVarNames[this] = name;
		playerVarTypes[this] = PlayerVarType::NONE;
	}
    const char* PlayerVariable::GetName() {
        auto it = playerVarNames.find(this);
        if (it != playerVarNames.end()) {
            return it->second.c_str();
        }
        return nullptr;
    }
	void PlayerVariable::SetName(const std::string& newName) {
		playerVarNames[this] = newName;
	}
	PlayerVarType PlayerVariable::GetType() {
		auto it = playerVarTypes.find(this);
		if (it != playerVarTypes.end()) {
			return it->second;
		}
		return PlayerVarType::NONE;
	}
	void PlayerVariable::SetType(PlayerVarType newType) {
		playerVarTypes[this] = newType;
	}

	StringPlayerVariable::StringPlayerVariable(const std::string& name) : PlayerVariable(name) {
		SetType(PlayerVarType::String);
	}
	FloatPlayerVariable::FloatPlayerVariable(const std::string& name) : PlayerVariable(name) {
		SetType(PlayerVarType::Float);
	}
	BoolPlayerVariable::BoolPlayerVariable(const std::string& name) : PlayerVariable(name) {
		SetType(PlayerVarType::Bool);
	}

	std::vector<std::unique_ptr<PlayerVariable>> PlayerVariables::playerVars{};
	std::vector<std::unique_ptr<PlayerVariable>> PlayerVariables::defaultPlayerVars{};
	std::vector<std::unique_ptr<PlayerVariable>> PlayerVariables::customDefaultPlayerVars{};
	bool PlayerVariables::gotPlayerVars = false;
	static bool sortedPlayerVars = false;

	std::unordered_map<std::string, std::any> PlayerVariables::prevPlayerVarValueMap{};
	std::unordered_map<std::string, bool> PlayerVariables::prevBoolValueMap{};

	template <typename T>
	static void updateDefaultVar(std::vector<std::unique_ptr<PlayerVariable>>& defaultVars, const std::string& name, T value, T defaultValue) {
		static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

		auto playerVarIt = std::find_if(defaultVars.begin(), defaultVars.end(), [&name](const auto& playerVar) {
			return playerVar->GetName() == name;
		});
		if (playerVarIt == defaultVars.end()) {
            if constexpr (std::is_same_v<T, std::string>) {
				auto stringPlayerVar = std::make_unique<StringPlayerVariable>(name);
				stringPlayerVar->value = value;
				stringPlayerVar->defaultValue = defaultValue;
				defaultVars.emplace_back(std::move(stringPlayerVar));
            }
            else if constexpr (std::is_same_v<T, float>) {
				auto floatPlayerVar = std::make_unique<FloatPlayerVariable>(name);
				floatPlayerVar->value = value;
				floatPlayerVar->defaultValue = defaultValue;
				defaultVars.emplace_back(std::move(floatPlayerVar));
            }
            else if constexpr (std::is_same_v<T, bool>) {
				auto boolPlayerVar = std::make_unique<BoolPlayerVariable>(name);
				boolPlayerVar->value = value;
				boolPlayerVar->defaultValue = defaultValue;
				defaultVars.emplace_back(std::move(boolPlayerVar));
            }
		} else {
			if constexpr (std::is_same_v<T, std::string>) {
				// TO IMPLEMENT
				return;
			} else if constexpr (std::is_same_v<T, float>) {
				auto floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVarIt->get());
				floatPlayerVar->value = value;
				floatPlayerVar->defaultValue = defaultValue;
			} else if constexpr (std::is_same_v<T, bool>) {
				auto boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVarIt->get());
				boolPlayerVar->value = value;
				boolPlayerVar->defaultValue = defaultValue;
			}
		}
	}
	static void processPlayerVar(DWORD64*(*playerVarsGetter)(), std::unique_ptr<PlayerVariable>& playerVar) {
		static int offset = 0;
		int offsetDif = 0;
		while (true) {
			std::string vTableName = Utils::RTTI::GetVTableName(playerVarsGetter() + offset);
			if (vTableName != "StringPlayerVariable" && vTableName != "FloatPlayerVariable" && vTableName != "BoolPlayerVariable") {
				if (offsetDif > 150)
					return;

				offset += 1;
				offsetDif += 1;
				continue;
			}

			std::string varName = playerVar->GetName();
			PlayerVarType varType = playerVar->GetType();

			switch (playerVar->GetType()) {
			case PlayerVarType::String: {
				if (vTableName != "StringPlayerVariable")
					return;

				StringPlayerVariable* stringPlayerVar = reinterpret_cast<StringPlayerVariable*>(playerVarsGetter() + offset);
				playerVar.reset(stringPlayerVar);
				playerVar->SetName(varName);
				playerVar->SetType(varType);
				// TO IMPLEMENT

				offset += STRING_SIZE_OFFSET;
				return;
			}
			case PlayerVarType::Float: {
				if (vTableName != "FloatPlayerVariable")
					return;

				FloatPlayerVariable* floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVarsGetter() + offset);
				playerVar.reset(floatPlayerVar);
				playerVar->SetName(varName);
				playerVar->SetType(varType);
				updateDefaultVar(PlayerVariables::defaultPlayerVars, varName, floatPlayerVar->value.data, floatPlayerVar->defaultValue.data);
				updateDefaultVar(PlayerVariables::customDefaultPlayerVars, varName, floatPlayerVar->value.data, floatPlayerVar->defaultValue.data);

				offset += FLOAT_SIZE_OFFSET;
				return;
			}
			case PlayerVarType::Bool: {
				if (vTableName != "BoolPlayerVariable")
					return;

				BoolPlayerVariable* boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVarsGetter() + offset);
				playerVar.reset(boolPlayerVar);
				playerVar->SetName(varName);
				playerVar->SetType(varType);
				updateDefaultVar(PlayerVariables::defaultPlayerVars, varName, boolPlayerVar->value.data, boolPlayerVar->defaultValue.data);
				updateDefaultVar(PlayerVariables::customDefaultPlayerVars, varName, boolPlayerVar->value.data, boolPlayerVar->defaultValue.data);

				offset += BOOL_SIZE_OFFSET;
				return;
			}
			default:
				offset += 1;
				return;
			}
		}
	}

	void PlayerVariables::GetPlayerVars() {
		if (gotPlayerVars)
			return;
		if (!sortedPlayerVars)
			return;
		if (!Get())
			return;

		for (auto& var : playerVars) {
			__try {
				processPlayerVar(reinterpret_cast<DWORD64*(*)()>(&Get), var);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				SPDLOG_ERROR("Failed to process player variable: {}", var->GetName());
			}
		}

		gotPlayerVars = true;
	}

#pragma region Player Variables Sorting
	struct VarTypeFieldMeta {
		PlayerVarType type;
		std::string_view className;
	};
	const std::vector<VarTypeFieldMeta> varTypeFields = {
		{ PlayerVarType::String, "constds::FieldsCollection<PlayerVariables>::TypedFieldMeta<StringPlayerVariable>" },
		{ PlayerVarType::Float, "constds::FieldsCollection<PlayerVariables>::TypedFieldMeta<FloatPlayerVariable>" },
		{ PlayerVarType::Bool, "constds::FieldsCollection<PlayerVariables>::TypedFieldMeta<BoolPlayerVariable>" }
	};

	static bool isRetInstruction(BYTE* address) {
		//return address[0] == 0xC3 && address[1] == 0xCC;
		return address[0] == 0x00 && address[1] == 0x00 && address[2] == 0xC3 && address[3] == 0xCC;
	}
	static bool isLeaInstruction(BYTE* address, BYTE REX, BYTE ModRM) {
		return address[0] == REX && address[1] == 0x8D && address[2] == ModRM;
	}
	static bool isCallInstruction(BYTE* address) {
		return address[0] == 0xE8 && address[4] != 0xE8;
	}
	static bool isBelowFuncSizeLimit(BYTE* address, DWORD64 startOfFunc, size_t sizeLimit) {
		return (reinterpret_cast<DWORD64>(address) - startOfFunc) < sizeLimit;
	}

	// to prevent infinite loops, assuming function is no longer than 500000 bytes LMAO Techland... why is your function even like 250000 bytes to begin with? bad code...
	static const size_t MAX_FUNC_SIZE = 500000;
	static const size_t MAX_LOAD_VAR_FUNC_SIZE = 2000;

	static const char* getPlayerVarName(BYTE*& funcAddress, DWORD64 startOfFunc) {
		const char* playerVarName = nullptr;
		while (!playerVarName && !isRetInstruction(funcAddress) && isBelowFuncSizeLimit(funcAddress, startOfFunc, MAX_FUNC_SIZE)) {
			// lea r8, varNameString
			if (!isLeaInstruction(funcAddress, 0x4C, 0x05)) {
				funcAddress++;
				continue;
			}

			playerVarName = reinterpret_cast<const char*>(Utils::Memory::CalcTargetAddrOfRelativeInstr(reinterpret_cast<DWORD64>(funcAddress), 3));
			if (!playerVarName) {
				funcAddress++;
				continue;
			}

			// add the size of the instruction, so we skip this instruction because this instruction is the name
			funcAddress += 0x7;
		}

		return playerVarName;
	}
	static PlayerVarType getPlayerVarType(BYTE*& funcAddress, DWORD64 startOfFunc) {
		PlayerVarType playerVarType = PlayerVarType::NONE;

		while (!playerVarType && !isRetInstruction(funcAddress) && isBelowFuncSizeLimit(funcAddress, startOfFunc, MAX_FUNC_SIZE)) {
			// call LoadPlayerXVariable
			if (!isCallInstruction(funcAddress)) {
				funcAddress++;
				continue;
			}

			DWORD64 startOfLoadVarFunc = Utils::Memory::CalcTargetAddrOfRelativeInstr(reinterpret_cast<DWORD64>(funcAddress), 1);
			BYTE* loadVarFuncAddress = reinterpret_cast<BYTE*>(startOfLoadVarFunc);
			DWORD64 metaVTAddrFromFunc = 0;

			while (!metaVTAddrFromFunc && !isRetInstruction(loadVarFuncAddress) && isBelowFuncSizeLimit(loadVarFuncAddress, startOfLoadVarFunc, MAX_LOAD_VAR_FUNC_SIZE)) {
				// lea rax, typedFieldMetaVT
				if (!isLeaInstruction(loadVarFuncAddress, 0x48, 0x05)) {
					loadVarFuncAddress++;
					continue;
				}

				metaVTAddrFromFunc = Utils::Memory::CalcTargetAddrOfRelativeInstr(reinterpret_cast<DWORD64>(loadVarFuncAddress), 3);
				std::string vTableName = Utils::RTTI::GetVTableNameFromVTPtr(reinterpret_cast<DWORD64*>(metaVTAddrFromFunc));
				auto varTypeIt = std::find_if(varTypeFields.begin(), varTypeFields.end(), [&vTableName](const auto& varType) {
					return varType.className == vTableName;
				});
				if (varTypeIt == varTypeFields.end()) {
					metaVTAddrFromFunc = 0;
					loadVarFuncAddress++;
					continue;
				}

				playerVarType = varTypeIt->type;
				break;
			}

			// if it's still NONE after seeing the function doesnt reference any of the variables, break so the loop stops
			if (playerVarType == PlayerVarType::NONE)
				break;
		}

		return playerVarType;
	}

	bool PlayerVariables::SortPlayerVars() {
		DWORD64 startOfFunc = 0;
		while (!startOfFunc)
			startOfFunc = reinterpret_cast<DWORD64>(Offsets::Get_LoadPlayerVars());

		BYTE* funcAddress = reinterpret_cast<BYTE*>(startOfFunc);
		while (!isRetInstruction(funcAddress) && (reinterpret_cast<DWORD64>(funcAddress) - startOfFunc) < MAX_FUNC_SIZE) {
			const char* playerVarName = getPlayerVarName(funcAddress, startOfFunc);
			if (!playerVarName)
				continue;

			PlayerVarType playerVarType = getPlayerVarType(funcAddress, startOfFunc);
			switch (playerVarType) {
			case PlayerVarType::String:
				playerVars.emplace_back(std::make_unique<StringPlayerVariable>(playerVarName));
				break;
			case PlayerVarType::Float:
				playerVars.emplace_back(std::make_unique<FloatPlayerVariable>(playerVarName));
				break;
			case PlayerVarType::Bool:
				playerVars.emplace_back(std::make_unique<BoolPlayerVariable>(playerVarName));
				break;
			default:
				//playerVars.emplace_back(std::make_unique<PlayerVariable>(playerVarName));
				break;
			}
		}

		sortedPlayerVars = true;
		return true;
	}
#pragma endregion

	static PlayerVariables* GetOffset_PlayerVariables() {
		PlayerState* playerState = PlayerState::Get();
		return playerState ? playerState->playerVariables : nullptr;
	}
	PlayerVariables* PlayerVariables::Get() {
		return ClassHelpers::SafeGetter<PlayerVariables>(GetOffset_PlayerVariables, false, false);
	}
}