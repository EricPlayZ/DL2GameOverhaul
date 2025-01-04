#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <spdlog\spdlog.h>
#include <EGSDK\Offsets.h>
#include <EGSDK\GamePH\PlayerState.h>
#include <EGSDK\GamePH\PlayerVariables.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	static const int FLOAT_VAR_OFFSET = 3;
	static const int BOOL_VAR_OFFSET = 2;
	static const int VAR_LOC_OFFSET = 1;

	std::vector<std::pair<std::string, std::pair<void*, std::string>>> PlayerVariables::playerVars;
	std::vector<std::pair<std::string, std::pair<std::any, std::string>>> PlayerVariables::playerVarsDefault;
	std::vector<std::pair<std::string, std::pair<std::any, std::string>>> PlayerVariables::playerCustomVarsDefault;
	bool PlayerVariables::gotPlayerVars = false;
	static bool sortedPlayerVars = false;

	template <typename T>
	static void updateDefaultVar(std::vector<std::pair<std::string, std::pair<std::any, std::string>>>& defaultVars, const std::string& varName, T varValue) {
		static_assert(std::is_same<T, float>::value || std::is_same<T, bool>::value, "Invalid type: value must be float or bool");

		auto it = std::find_if(defaultVars.begin(), defaultVars.end(), [&varName](const auto& pair) {
			return pair.first == varName;
		});
		if (it == defaultVars.end())
			return;

		it->second.first.template emplace<T>(varValue);
	}
	static void processPlayerVar(DWORD64*(*playerVarsGetter)(), std::pair<std::string, std::pair<void*, std::string>>& var) {
		static int offset = 0;
		static const std::string floatPlayerVarClassName = "FloatPlayerVariable";
		static const std::string boolPlayerVarClassName = "BoolPlayerVariable";

		while (true) {
			const std::string vTableName = Utils::RTTI::GetVTableName(playerVarsGetter() + offset);
			const bool isFloatPlayerVar = vTableName == floatPlayerVarClassName;
			const bool isBoolPlayerVar = vTableName == boolPlayerVarClassName;

			if (isFloatPlayerVar || isBoolPlayerVar) {
				var.second.first = playerVarsGetter() + offset + VAR_LOC_OFFSET;
				const std::string& varName = var.first;

				if (isFloatPlayerVar) {
					float* varValue = reinterpret_cast<float*>(var.second.first);
					updateDefaultVar(GamePH::PlayerVariables::playerVarsDefault, varName, *varValue);
					updateDefaultVar(GamePH::PlayerVariables::playerCustomVarsDefault, varName, *varValue);

					offset += FLOAT_VAR_OFFSET;
				} else {
					bool* varValue = reinterpret_cast<bool*>(var.second.first);
					updateDefaultVar(GamePH::PlayerVariables::playerVarsDefault, varName, *varValue);
					updateDefaultVar(GamePH::PlayerVariables::playerCustomVarsDefault, varName, *varValue);

					offset += BOOL_VAR_OFFSET;
				}

				break;
			} else
				offset += 1;
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
				SPDLOG_ERROR("Failed to process player variable: {}", var.first);
			}
		}

		gotPlayerVars = true;
	}

#pragma region Player Variables Sorting
	struct VarTypeFieldMeta {
		PlayerVariables::PlayerVarType type;
		std::string_view className;
	};
	const std::vector<VarTypeFieldMeta> varTypeFields = {
		{ PlayerVariables::PlayerVarType::Float, "constds::FieldsCollection<PlayerVariables>::TypedFieldMeta<FloatPlayerVariable>" },
		{ PlayerVariables::PlayerVarType::Bool, "constds::FieldsCollection<PlayerVariables>::TypedFieldMeta<BoolPlayerVariable>" }
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
	static PlayerVariables::PlayerVarType getPlayerVarType(BYTE*& funcAddress, DWORD64 startOfFunc) {
		PlayerVariables::PlayerVarType playerVarType = PlayerVariables::PlayerVarType::NONE;

		while (!playerVarType && !isRetInstruction(funcAddress) && isBelowFuncSizeLimit(funcAddress, startOfFunc, MAX_FUNC_SIZE)) {
			// call LoadPlayerXVariable
			if (!isCallInstruction(funcAddress)) {
				funcAddress++;
				continue;
			}

			DWORD64 startOfLoadVarFunc = Utils::Memory::CalcTargetAddrOfRelativeInstr(reinterpret_cast<DWORD64>(funcAddress), 1);
			for (const auto& varType : varTypeFields) {
				BYTE* loadVarFuncAddress = reinterpret_cast<BYTE*>(startOfLoadVarFunc);
				DWORD64 metaVTAddrFromFunc = 0;

				while (!metaVTAddrFromFunc && !isRetInstruction(loadVarFuncAddress) && isBelowFuncSizeLimit(loadVarFuncAddress, startOfLoadVarFunc, MAX_LOAD_VAR_FUNC_SIZE)) {
					// lea rax, typedFieldMetaVT
					if (!isLeaInstruction(loadVarFuncAddress, 0x48, 0x05)) {
						loadVarFuncAddress++;
						continue;
					}

					metaVTAddrFromFunc = Utils::Memory::CalcTargetAddrOfRelativeInstr(reinterpret_cast<DWORD64>(loadVarFuncAddress), 3);
					if (Utils::RTTI::GetVTableNameFromVTPtr(reinterpret_cast<DWORD64*>(metaVTAddrFromFunc)) != varType.className) {
						metaVTAddrFromFunc = 0;
						loadVarFuncAddress++;
						continue;
					}
				}

				if (Utils::RTTI::GetVTableNameFromVTPtr(reinterpret_cast<DWORD64*>(metaVTAddrFromFunc)) == varType.className) {
					playerVarType = varType.type;
					break;
				}
			}

			// if it's still NONE after seeing the function doesnt reference any of the variables, break so the loop stops
			if (playerVarType == PlayerVariables::PlayerVarType::NONE)
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
			if (!playerVarType)
				continue;

			std::string varType{};
			switch (playerVarType) {
			case PlayerVarType::Float:
				varType = "float";
				break;
			case PlayerVarType::Bool:
				varType = "bool";
				break;
			default:
				break;
			}

			PlayerVariables::playerVars.emplace_back(playerVarName, std::make_pair(nullptr, varType));
			PlayerVariables::playerVarsDefault.emplace_back(playerVarName, std::make_pair(varType == "float" ? 0.0f : false, varType));
			PlayerVariables::playerCustomVarsDefault.emplace_back(playerVarName, std::make_pair(varType == "float" ? 0.0f : false, varType));
		}

		sortedPlayerVars = true;
		return true;
	}
#pragma endregion

	std::unordered_map<std::string, std::any> PlayerVariables::prevPlayerVarValueMap{};
	std::unordered_map<std::string, bool> PlayerVariables::prevBoolValueMap{};

	static PlayerVariables* GetOffset_PlayerVariables() {
		PlayerState* pPlayerState = PlayerState::Get();
		return pPlayerState ? pPlayerState->pPlayerVariables : nullptr;
	}
	PlayerVariables* PlayerVariables::Get() {
		return ClassHelpers::SafeGetter<PlayerVariables>(GetOffset_PlayerVariables, false, false);
	}
}