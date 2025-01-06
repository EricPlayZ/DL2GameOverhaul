#pragma once
#include <any>
#include <unordered_map>
#include <vector>
#include <memory>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\Values.h>

namespace EGSDK::GamePH {
	enum EGameSDK_API PlayerVarType {
		NONE = 0,
		String,
		Float,
		Bool
	};

	class EGameSDK_API PlayerVariable {
	public:
		PlayerVariable(const std::string& name);

		const char* GetName();
		void SetName(const std::string& newName);

		PlayerVarType GetType();
		void SetType(PlayerVarType newType);
	private:
		static std::unordered_map<PlayerVariable*, std::string> playerVarNames;
		static std::unordered_map<PlayerVariable*, PlayerVarType> playerVarTypes;
	};
	class EGameSDK_API StringPlayerVariable : public PlayerVariable {
	public:
		union {
			EGSDK::ClassHelpers::buffer<0x8, const char*> value; // remove 0x2 bit to access ptr
			EGSDK::ClassHelpers::buffer<0x10, const char*> defaultValue; // remove 0x2 bit to access ptr
		};
		StringPlayerVariable(const std::string& name);
	};
	class EGameSDK_API FloatPlayerVariable : public PlayerVariable {
	public:
		union {
			EGSDK::ClassHelpers::buffer<0x8, float> value;
			EGSDK::ClassHelpers::buffer<0xC, float> defaultValue;
		};
		FloatPlayerVariable(const std::string& name);
	};
	class EGameSDK_API BoolPlayerVariable : public PlayerVariable {
	public:
		union {
			EGSDK::ClassHelpers::buffer<0x8, bool> value;
			EGSDK::ClassHelpers::buffer<0x9, bool> defaultValue;
		};
		BoolPlayerVariable(const std::string& name);
	};

	class EGameSDK_API PlayerVariables {
	public:
		static std::vector<std::unique_ptr<PlayerVariable>> playerVars;
		static std::vector<std::unique_ptr<PlayerVariable>> defaultPlayerVars;
		static std::vector<std::unique_ptr<PlayerVariable>> customDefaultPlayerVars;
		static bool gotPlayerVars;

		static std::unordered_map<std::string, std::any> prevPlayerVarValueMap;
		static std::unordered_map<std::string, bool> prevBoolValueMap;

#ifdef EGameSDK_EXPORTS
		static void GetPlayerVars();
		static bool SortPlayerVars();
#endif

		template <typename T>
		static T getDefaultValue() {
			static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

			if constexpr (std::is_same_v<T, std::string>)
				return {};
			else if constexpr (std::is_same_v<T, float>)
				return -404.0f;
			else if constexpr (std::is_same_v<T, bool>)
				return false;
			else
				return T();
		}

		template <typename T>
		static T GetPlayerVarValue(const std::string& name) {
			static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

			if (!gotPlayerVars)
				return getDefaultValue<T>();

			auto playerVarIt = std::find_if(playerVars.begin(), playerVars.end(), [&name](const auto& playerVar) {
				return playerVar->GetName() == name;
			});
			if (playerVarIt == playerVars.end())
				return getDefaultValue<T>();

			auto playerVar = playerVarIt->get();
			if constexpr (std::is_same_v<T, std::string>) {
				StringPlayerVariable* stringPlayerVar = reinterpret_cast<StringPlayerVariable*>(playerVar);
				return stringPlayerVar->value.data;
			} else if constexpr (std::is_same_v<T, float>) {
				FloatPlayerVariable* floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVar);
				return floatPlayerVar->value.data;
			} else if constexpr (std::is_same_v<T, bool>) {
				BoolPlayerVariable* boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVar);
				return boolPlayerVar->value.data;
			}
		}
		template <typename T>
		static void ChangePlayerVar(const std::string& name, const T value) {
			static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

			if (!gotPlayerVars)
				return;

			auto playerVarIt = std::find_if(playerVars.begin(), playerVars.end(), [&name](const auto& playerVar) {
				return playerVar->GetName() == name;
			});
			if (playerVarIt == playerVars.end())
				return;

			auto playerVar = playerVarIt->get();
			if constexpr (std::is_same_v<T, std::string>) {
				switch (playerVar->GetType()) {
				case PlayerVarType::String:
					// TO IMPLEMENT
					break;
				case PlayerVarType::Float: {
					std::string valueStr = Utils::Values::to_string(value);
					float actualValue = std::stof(valueStr);

					FloatPlayerVariable* floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVar);
					floatPlayerVar->value = actualValue;
					floatPlayerVar->defaultValue = actualValue;
					break;
				}
				case PlayerVarType::Bool: {
					std::string valueStr = Utils::Values::to_string(value);
					bool actualValue = valueStr == "true";

					BoolPlayerVariable* boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVar);
					boolPlayerVar->value = actualValue;
					boolPlayerVar->defaultValue = actualValue;
					break;
				}
				default:
					break;
				}
			} else if constexpr (std::is_same_v<T, float>) {
				if (playerVar->GetType() != PlayerVarType::Float)
					return;

				FloatPlayerVariable* floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVar);
				floatPlayerVar->value = value;
				floatPlayerVar->defaultValue = value;
			} else if constexpr (std::is_same_v<T, bool>) {
				if (playerVar->GetType() != PlayerVarType::Bool)
					return;

				BoolPlayerVariable* boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVar);
				boolPlayerVar->value = value;
				boolPlayerVar->defaultValue = value;
			}
		}

		template <typename T>
		static void ManagePlayerVarByBool(const std::string& playerVar, const T valueIfTrue, const T valueIfFalse, bool boolVal, bool usePreviousVal = true) {
			if (!gotPlayerVars)
				return;

			if (prevPlayerVarValueMap.find(playerVar) == prevPlayerVarValueMap.end())
				prevPlayerVarValueMap[playerVar] = GetPlayerVarValue<T>(playerVar);
			if (prevBoolValueMap.find(playerVar) == prevBoolValueMap.end())
				prevBoolValueMap[playerVar] = false;

			if (boolVal) {
				if (!prevBoolValueMap[playerVar])
					prevPlayerVarValueMap[playerVar] = GetPlayerVarValue<T>(playerVar);

				ChangePlayerVar(playerVar, valueIfTrue);
				prevBoolValueMap[playerVar] = true;
			} else if (prevBoolValueMap[playerVar]) {
				prevBoolValueMap[playerVar] = false;
				ChangePlayerVar(playerVar, usePreviousVal ? std::any_cast<T>(prevPlayerVarValueMap[playerVar]) : valueIfFalse);
			}
		}

		static PlayerVariables* Get();
	};
}