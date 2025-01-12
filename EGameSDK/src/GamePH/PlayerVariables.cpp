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
	std::mutex PlayerVariable::mutex{};

	PlayerVariable::PlayerVariable(const std::string& name) {
		std::lock_guard<std::mutex> lock(mutex);
		playerVarNames[this] = name;
		playerVarTypes[this] = PlayerVarType::NONE;
	}
    const char* PlayerVariable::GetName() {
		std::lock_guard<std::mutex> lock(mutex);
        auto it = playerVarNames.find(this);
        if (it != playerVarNames.end()) {
            return it->second.c_str();
        }
        return nullptr;
    }
	void PlayerVariable::SetName(const std::string& newName) {
		std::lock_guard<std::mutex> lock(mutex);
		playerVarNames[this] = newName;
	}
	PlayerVarType PlayerVariable::GetType() {
		std::lock_guard<std::mutex> lock(mutex);
		auto it = playerVarTypes.find(this);
		if (it != playerVarTypes.end()) {
			return it->second;
		}
		return PlayerVarType::NONE;
	}
	void PlayerVariable::SetType(PlayerVarType newType) {
		std::lock_guard<std::mutex> lock(mutex);
		playerVarTypes[this] = newType;
	}

	StringPlayerVariable::StringPlayerVariable(const std::string& name) : PlayerVariable(name) {
		SetType(PlayerVarType::String);
	}
	const char* StringPlayerVariable::GetValue() {
		return reinterpret_cast<const char*>(reinterpret_cast<DWORD64>(this->value.data) & 0x1FFFFFFFFFFFFFFF);
	}
	const char* StringPlayerVariable::GetDefaultValue() {
		return reinterpret_cast<const char*>(reinterpret_cast<DWORD64>(this->defaultValue.data) & 0x1FFFFFFFFFFFFFFF);
	}
	void StringPlayerVariable::SetValues(const std::string& value) {
		DWORD64 firstByte = (reinterpret_cast<DWORD64>(this->value.data) >> 56) & 0xFF;
		DWORD64 newValueAddr = reinterpret_cast<DWORD64>(value.c_str());
		if (firstByte != 0x0)
			newValueAddr |= (firstByte << 56);

		this->value = reinterpret_cast<const char*>(newValueAddr);
		this->defaultValue = reinterpret_cast<const char*>(newValueAddr);
	}
	FloatPlayerVariable::FloatPlayerVariable(const std::string& name) : PlayerVariable(name) {
		SetType(PlayerVarType::Float);
	}
	float FloatPlayerVariable::GetValue() {
		return this->value.data;
	}
	float FloatPlayerVariable::GetDefaultValue() {
		return this->defaultValue.data;
	}
	void FloatPlayerVariable::SetValues(float value) {
		this->value = value;
		this->defaultValue = value;
	}
	BoolPlayerVariable::BoolPlayerVariable(const std::string& name) : PlayerVariable(name) {
		SetType(PlayerVarType::Bool);
	}
	bool BoolPlayerVariable::GetValue() {
		return this->value.data;
	}
	bool BoolPlayerVariable::GetDefaultValue() {
		return this->defaultValue.data;
	}
	void BoolPlayerVariable::SetValues(bool value) {
		this->value = value;
		this->defaultValue = value;
	}

	std::unique_ptr<PlayerVariable>& PlayerVarMap::try_emplace(std::unique_ptr<PlayerVariable> playerVar) {
		std::lock_guard<std::mutex> lock(_mutex);
		const std::string& name = playerVar->GetName();
		auto [it, inserted] = _playerVars.try_emplace(name, std::move(playerVar));
		if (inserted)
			_order.emplace_back(name);
		return it->second;
	}
	bool PlayerVarMap::empty() {
		std::lock_guard<std::mutex> lock(_mutex);
		return _playerVars.empty();
	}
	bool PlayerVarMap::none_of(const std::string& name) {
		std::lock_guard<std::mutex> lock(_mutex);
		return _playerVars.find(name) == _playerVars.end();
	}

	std::unique_ptr<PlayerVariable>* PlayerVarMap::FindPtr(const std::string& name) {
		std::lock_guard<std::mutex> lock(_mutex);
		auto it = _playerVars.find(name);
		return it == _playerVars.end() ? nullptr : &it->second;
	}
	PlayerVariable* PlayerVarMap::Find(const std::string& name) {
		std::lock_guard<std::mutex> lock(_mutex);
		auto it = _playerVars.find(name);
		return it == _playerVars.end() ? nullptr : it->second.get();
	}
	bool PlayerVarMap::Erase(const std::string& name) {
		std::lock_guard<std::mutex> lock(_mutex);
		return _playerVars.erase(name) > 0;
	}

	PlayerVarMap PlayerVariables::playerVars{};
	PlayerVarMap PlayerVariables::customPlayerVars{};
	PlayerVarMap PlayerVariables::defaultPlayerVars{};
	PlayerVarMap PlayerVariables::customDefaultPlayerVars{};
	std::atomic<bool> PlayerVariables::gotPlayerVars = false;
	static bool sortedPlayerVars = false;

	std::unordered_map<std::string, std::any> PlayerVariables::prevPlayerVarValueMap{};
	std::unordered_map<std::string, bool> PlayerVariables::prevBoolValueMap{};

	template <typename T>
	static void updateDefaultVar(PlayerVarMap& defaultVars, const std::string& name, T value, T defaultValue) {
		static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

		auto playerVar = defaultVars.Find(name);
		if (!playerVar) {
            if constexpr (std::is_same_v<T, std::string>) {
				auto stringPlayerVar = std::make_unique<StringPlayerVariable>(name);
				defaultVars.try_emplace(std::move(stringPlayerVar));
            }
            else if constexpr (std::is_same_v<T, float>) {
				auto floatPlayerVar = std::make_unique<FloatPlayerVariable>(name);
				floatPlayerVar->SetValues(value);
				defaultVars.try_emplace(std::move(floatPlayerVar));
            }
            else if constexpr (std::is_same_v<T, bool>) {
				auto boolPlayerVar = std::make_unique<BoolPlayerVariable>(name);
				boolPlayerVar->SetValues(value);
				defaultVars.try_emplace(std::move(boolPlayerVar));
            }
		} else {
			if constexpr (std::is_same_v<T, std::string>) {
				// TO IMPLEMENT
				return;
			} else if constexpr (std::is_same_v<T, float>) {
				auto floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVar);
				floatPlayerVar->SetValues(value);
			} else if constexpr (std::is_same_v<T, bool>) {
				auto boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVar);
				boolPlayerVar->SetValues(value);
			}
		}
	}
	static void processPlayerVar(DWORD64*(*playerVarsGetter)(), std::unique_ptr<PlayerVariable>& playerVarPtr) {
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

			std::string varName = playerVarPtr->GetName();
			PlayerVarType varType = playerVarPtr->GetType();

			switch (playerVarPtr->GetType()) {
			case PlayerVarType::String:
			{
				if (vTableName != "StringPlayerVariable")
					return;

				StringPlayerVariable* stringPlayerVar = reinterpret_cast<StringPlayerVariable*>(playerVarsGetter() + offset);
				playerVarPtr.reset(stringPlayerVar);
				playerVarPtr->SetName(varName);
				playerVarPtr->SetType(varType);
				// TO IMPLEMENT

				offset += STRING_SIZE_OFFSET;
				return;
			}
			case PlayerVarType::Float:
			{
				if (vTableName != "FloatPlayerVariable")
					return;

				FloatPlayerVariable* floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVarsGetter() + offset);
				playerVarPtr.reset(floatPlayerVar);
				playerVarPtr->SetName(varName);
				playerVarPtr->SetType(varType);
				updateDefaultVar(PlayerVariables::customDefaultPlayerVars, varName, floatPlayerVar->value.data, floatPlayerVar->defaultValue.data);

				offset += FLOAT_SIZE_OFFSET;
				return;
			}
			case PlayerVarType::Bool:
			{
				if (vTableName != "BoolPlayerVariable")
					return;

				BoolPlayerVariable* boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVarsGetter() + offset);
				playerVarPtr.reset(boolPlayerVar);
				playerVarPtr->SetName(varName);
				playerVarPtr->SetType(varType);
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
	static void processPlayerVarSafe(std::unique_ptr<PlayerVariable>& playerVarPtr, DWORD64*(*playerVarsGetter)()) {
		__try {
			processPlayerVar(playerVarsGetter, playerVarPtr);
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			SPDLOG_ERROR("Failed to process player variable: {}", playerVarPtr->GetName());
		}
	}

	void PlayerVariables::GetPlayerVars() {
		if (gotPlayerVars)
			return;
		if (!sortedPlayerVars)
			return;
		if (!Get())
			return;

		playerVars.ForEach(processPlayerVarSafe, reinterpret_cast<DWORD64*(*)()>(&Get));
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
				playerVars.try_emplace(std::make_unique<StringPlayerVariable>(playerVarName));
				break;
			case PlayerVarType::Float:
				playerVars.try_emplace(std::make_unique<FloatPlayerVariable>(playerVarName));
				break;
			case PlayerVarType::Bool:
				playerVars.try_emplace(std::make_unique<BoolPlayerVariable>(playerVarName));
				break;
			default:
				//playerVars.try_emplace(std::make_unique<PlayerVariable>(playerVarName));
				break;
			}
		}

		sortedPlayerVars = true;
		return true;
	}
#pragma endregion

	template <typename T>
	static T getDefaultValue() {
		static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

		if constexpr (std::is_same_v<T, std::string>)
			return {};
		else if constexpr (std::is_same_v<T, float>)
			return -404.0f;
		else if constexpr (std::is_same_v<T, bool>)
			return false;
	}
	template <typename T>
	T PlayerVariables::GetPlayerVarValue(const std::string& name) {
		static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

		if (!gotPlayerVars)
			return getDefaultValue<T>();

		auto playerVar = playerVars.Find(name);
		if (!playerVar)
			return getDefaultValue<T>();

		if constexpr (std::is_same_v<T, std::string>)
			return reinterpret_cast<StringPlayerVariable*>(playerVar)->value.data;
		else if constexpr (std::is_same_v<T, float>)
			return reinterpret_cast<FloatPlayerVariable*>(playerVar)->value.data;
		else if constexpr (std::is_same_v<T, bool>)
			return reinterpret_cast<BoolPlayerVariable*>(playerVar)->value.data;
	}
	template <typename T>
	void PlayerVariables::ChangePlayerVar(const std::string& name, const T value, PlayerVariable* playerVar) {
		static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

		if (!gotPlayerVars)
			return;

		if (!playerVar)
			playerVar = playerVars.Find(name);
		if (!playerVar)
			return;

		if constexpr (std::is_same_v<T, std::string>) {
			switch (playerVar->GetType()) {
				case PlayerVarType::String:
					break; // TO IMPLEMENT
				case PlayerVarType::Float:
					ChangePlayerVar<float>(name, std::stof(Utils::Values::to_string(value)), playerVar);
					break;
				case PlayerVarType::Bool:
					ChangePlayerVar<bool>(name, std::stof(Utils::Values::to_string(value)), playerVar);
					break;
				default:
					break;
			}
		} else if constexpr (std::is_same_v<T, float>) {
			if (playerVar->GetType() == PlayerVarType::Float)
				reinterpret_cast<FloatPlayerVariable*>(playerVar)->SetValues(value);
		} else if constexpr (std::is_same_v<T, bool>) {
			if (playerVar->GetType() == PlayerVarType::Bool)
				reinterpret_cast<BoolPlayerVariable*>(playerVar)->SetValues(value);
		}
	}
	template <typename T>
	void PlayerVariables::ChangePlayerVarFromList(const std::string& name, const T value, PlayerVariable* playerVar) {
		static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

		if (!gotPlayerVars)
			return;

		if (!playerVar)
			playerVar = playerVars.Find(name);
		if (!playerVar)
			return;

		auto customPlayerVar = customPlayerVars.Find(name);
		auto defPlayerVar = defaultPlayerVars.Find(name);

		if constexpr (std::is_same_v<T, std::string>) {
			switch (playerVar->GetType()) {
				case PlayerVarType::String:
					break; // TO IMPLEMENT
				case PlayerVarType::Float:
					ChangePlayerVarFromList<float>(name, std::stof(Utils::Values::to_string(value)), playerVar);
					break;
				case PlayerVarType::Bool:
					ChangePlayerVarFromList<bool>(name, std::stof(Utils::Values::to_string(value)), playerVar);
					break;
				default:
					break;
			}
		} else if constexpr (std::is_same_v<T, float>) {
			if (playerVar->GetType() != PlayerVarType::Float)
				return;
			if (!customPlayerVar)
				customPlayerVar = customPlayerVars.try_emplace(std::make_unique<FloatPlayerVariable>(name)).get();
			if (!defPlayerVar) {
				defPlayerVar = defaultPlayerVars.try_emplace(std::make_unique<FloatPlayerVariable>(name)).get();
				reinterpret_cast<FloatPlayerVariable*>(defPlayerVar)->SetValues(reinterpret_cast<FloatPlayerVariable*>(playerVar)->value);
			}

			reinterpret_cast<FloatPlayerVariable*>(playerVar)->SetValues(value);
			reinterpret_cast<FloatPlayerVariable*>(customPlayerVar)->SetValues(value);
		} else if constexpr (std::is_same_v<T, bool>) {
			if (playerVar->GetType() != PlayerVarType::Bool)
				return;
			if (!customPlayerVar)
				customPlayerVar = customPlayerVars.try_emplace(std::make_unique<BoolPlayerVariable>(name)).get();
			if (!defPlayerVar) {
				defPlayerVar = defaultPlayerVars.try_emplace(std::make_unique<BoolPlayerVariable>(name)).get();
				reinterpret_cast<BoolPlayerVariable*>(defPlayerVar)->SetValues(reinterpret_cast<BoolPlayerVariable*>(playerVar)->value);
			}

			reinterpret_cast<BoolPlayerVariable*>(playerVar)->SetValues(value);
			reinterpret_cast<BoolPlayerVariable*>(customPlayerVar)->SetValues(value);
		}
	}

	template <typename T>
	void PlayerVariables::ManagePlayerVarByBool(const std::string& name, const T valueIfTrue, const T valueIfFalse, bool boolVal, bool usePreviousVal) {
		if (!gotPlayerVars)
			return;

		if (prevPlayerVarValueMap.find(name) == prevPlayerVarValueMap.end())
			prevPlayerVarValueMap[name] = GetPlayerVarValue<T>(name);
		if (prevBoolValueMap.find(name) == prevBoolValueMap.end())
			prevBoolValueMap[name] = false;

		if (boolVal) {
			if (!prevBoolValueMap[name])
				prevPlayerVarValueMap[name] = GetPlayerVarValue<T>(name);

			ChangePlayerVar(name, valueIfTrue);
			prevBoolValueMap[name] = true;
		} else if (prevBoolValueMap[name]) {
			prevBoolValueMap[name] = false;
			ChangePlayerVar(name, usePreviousVal ? std::any_cast<T>(prevPlayerVarValueMap[name]) : valueIfFalse);
			prevPlayerVarValueMap.erase(name);
		}
	}
	bool PlayerVariables::IsPlayerVarManagedByBool(const std::string& name) {
		if (!gotPlayerVars)
			return false;

		return prevBoolValueMap.find(name) != prevBoolValueMap.end() && prevBoolValueMap[name];
	}

	template EGameSDK_API std::string PlayerVariables::GetPlayerVarValue<std::string>(const std::string& name);
	template EGameSDK_API float PlayerVariables::GetPlayerVarValue<float>(const std::string& name);
	template EGameSDK_API bool PlayerVariables::GetPlayerVarValue<bool>(const std::string& name);
	template EGameSDK_API void PlayerVariables::ChangePlayerVar<std::string>(const std::string& name, const std::string value, PlayerVariable* playerVar);
	template EGameSDK_API void PlayerVariables::ChangePlayerVar<float>(const std::string& name, const float value, PlayerVariable* playerVar);
	template EGameSDK_API void PlayerVariables::ChangePlayerVar<bool>(const std::string& name, const bool value, PlayerVariable* playerVar);
	template EGameSDK_API void PlayerVariables::ChangePlayerVarFromList<std::string>(const std::string& name, const std::string value, PlayerVariable* playerVar);
	template EGameSDK_API void PlayerVariables::ChangePlayerVarFromList<float>(const std::string& name, const float value, PlayerVariable* playerVar);
	template EGameSDK_API void PlayerVariables::ChangePlayerVarFromList<bool>(const std::string& name, const bool value, PlayerVariable* playerVar);

	template EGameSDK_API void PlayerVariables::ManagePlayerVarByBool(const std::string& name, const std::string valueIfTrue, const std::string valueIfFalse, bool boolVal, bool usePreviousVal);
	template EGameSDK_API void PlayerVariables::ManagePlayerVarByBool<float>(const std::string& name, const float valueIfTrue, const float valueIfFalse, bool boolVal, bool usePreviousVal);
	template EGameSDK_API void PlayerVariables::ManagePlayerVarByBool<bool>(const std::string& name, const bool valueIfTrue, const bool valueIfFalse, bool boolVal, bool usePreviousVal);

	static PlayerVariables* GetOffset_PlayerVariables() {
		PlayerState* playerState = PlayerState::Get();
		return playerState ? playerState->playerVariables : nullptr;
	}
	PlayerVariables* PlayerVariables::Get() {
		return ClassHelpers::SafeGetter<PlayerVariables>(GetOffset_PlayerVariables, false, false);
	}
}