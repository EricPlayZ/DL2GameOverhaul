#pragma once
#include <any>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
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

		void SetValues(float value);
	};
	class EGameSDK_API BoolPlayerVariable : public PlayerVariable {
	public:
		union {
			EGSDK::ClassHelpers::buffer<0x8, bool> value;
			EGSDK::ClassHelpers::buffer<0x9, bool> defaultValue;
		};
		BoolPlayerVariable(const std::string& name);

		void SetValues(bool value);
	};

	class EGameSDK_API PlayerVarVector {
	public:
		PlayerVarVector() = default;
		PlayerVarVector(const PlayerVarVector&) = delete; // Prevent copying
		PlayerVarVector& operator=(const PlayerVarVector&) = delete; // Prevent assignment

		PlayerVarVector(PlayerVarVector&&) = default; // Allow moving
		PlayerVarVector& operator=(PlayerVarVector&&) = default; // Allow move assignment

		std::unique_ptr<PlayerVariable>& emplace_back(std::unique_ptr<PlayerVariable> playerVar) {
			_playerVars.emplace_back(std::move(playerVar));
			return _playerVars.back();
		}
		auto begin() {
			return _playerVars.begin();
		}
		auto end() {
			return _playerVars.end();
		}
		bool none_of(const std::string& name) {
			return std::none_of(_playerVars.begin(), _playerVars.end(), [&name](const auto& playerVar) {
				return playerVar->GetName() == name;
			});
		}

		auto FindIter(const std::string& name) {
			auto playerVarIt = std::find_if(_playerVars.begin(), _playerVars.end(), [&name](const auto& playerVar) {
				return playerVar->GetName() == name;
			});
			return playerVarIt;
		}
		std::unique_ptr<PlayerVariable>* FindPtr(const std::string& name) {
			auto playerVarIt = FindIter(name);
			return playerVarIt == _playerVars.end() ? nullptr : &*playerVarIt;
		}
		PlayerVariable* Find(const std::string& name) {
			auto playerVarPtr = FindPtr(name);
			return playerVarPtr == nullptr ? nullptr : playerVarPtr->get();
		}
		auto Erase(const std::string& name) {
			auto playerVarIt = FindIter(name);
			if (playerVarIt != _playerVars.end())
				return _playerVars.erase(playerVarIt);
			return _playerVars.end();
		}

		std::vector<std::unique_ptr<PlayerVariable>> _playerVars{};
	};

	class EGameSDK_API PlayerVariables {
	public:
		static PlayerVarVector playerVars;
		static PlayerVarVector customPlayerVars;
		static PlayerVarVector defaultPlayerVars;
		static PlayerVarVector customDefaultPlayerVars;
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

			auto playerVar = playerVars.Find(name);
			if (!playerVar)
				return getDefaultValue<T>();

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

			auto playerVar = playerVars.Find(name);
			if (!playerVar)
				return;

			if constexpr (std::is_same_v<T, std::string>) {
				switch (playerVar->GetType()) {
				case PlayerVarType::String:
					// TO IMPLEMENT
					break;
				case PlayerVarType::Float:
				{
					std::string valueStr = Utils::Values::to_string(value);
					float actualValue = std::stof(valueStr);

					FloatPlayerVariable* floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVar);
					floatPlayerVar->SetValues(actualValue);
					break;
				}
				case PlayerVarType::Bool:
				{
					std::string valueStr = Utils::Values::to_string(value);
					bool actualValue = valueStr == "true";

					BoolPlayerVariable* boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVar);
					boolPlayerVar->SetValues(actualValue);
					break;
				}
				default:
					break;
				}
			} else if constexpr (std::is_same_v<T, float>) {
				if (playerVar->GetType() != PlayerVarType::Float)
					return;

				FloatPlayerVariable* floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVar);
				floatPlayerVar->SetValues(value);
			} else if constexpr (std::is_same_v<T, bool>) {
				if (playerVar->GetType() != PlayerVarType::Bool)
					return;

				BoolPlayerVariable* boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVar);
				boolPlayerVar->SetValues(value);
			}
		}
		template <typename T>
		static void ChangePlayerVarFromList(const std::string& name, const T value, PlayerVariable* playerVar = nullptr) {
			static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, bool>, "Invalid type: value must be string, float or bool");

			if (!gotPlayerVars)
				return;

			if (!playerVar) {
				playerVar = playerVars.Find(name);
				if (!playerVar)
					return;
			}

			auto customPlayerVar = customPlayerVars.Find(name);
			auto defPlayerVar = defaultPlayerVars.Find(name);

			if constexpr (std::is_same_v<T, std::string>) {
				switch (playerVar->GetType()) {
				case PlayerVarType::String:
					// TO IMPLEMENT
					break;
				case PlayerVarType::Float:
				{
					if (!customPlayerVar)
						customPlayerVar = customPlayerVars.emplace_back(std::make_unique<FloatPlayerVariable>(name)).get();
					std::string valueStr = Utils::Values::to_string(value);
					float actualValue = std::stof(valueStr);

					FloatPlayerVariable* floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVar);
					FloatPlayerVariable* customFloatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(customPlayerVar);

					if (!defPlayerVar) {
						defPlayerVar = defaultPlayerVars.emplace_back(std::make_unique<FloatPlayerVariable>(name)).get();

						FloatPlayerVariable* defFloatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(defPlayerVar);
						defFloatPlayerVar->SetValues(floatPlayerVar->value);
					}

					floatPlayerVar->SetValues(actualValue);
					customFloatPlayerVar->SetValues(actualValue);
					break;
				}
				case PlayerVarType::Bool:
				{
					if (!customPlayerVar)
						customPlayerVar = customPlayerVars.emplace_back(std::make_unique<BoolPlayerVariable>(name)).get();
					std::string valueStr = Utils::Values::to_string(value);
					bool actualValue = valueStr == "true";

					BoolPlayerVariable* boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVar);
					BoolPlayerVariable* customBoolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(customPlayerVar);

					if (!defPlayerVar) {
						defPlayerVar = defaultPlayerVars.emplace_back(std::make_unique<BoolPlayerVariable>(name)).get();

						BoolPlayerVariable* defBoolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(defPlayerVar);
						defBoolPlayerVar->SetValues(boolPlayerVar->value);
					}

					boolPlayerVar->SetValues(actualValue);
					customBoolPlayerVar->SetValues(actualValue);
					break;
				}
				default:
					break;
				}
			} else if constexpr (std::is_same_v<T, float>) {
				if (playerVar->GetType() != PlayerVarType::Float)
					return;
				if (!customPlayerVar)
					customPlayerVar = customPlayerVars.emplace_back(std::make_unique<FloatPlayerVariable>(name)).get();
				
				FloatPlayerVariable* floatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(playerVar);
				FloatPlayerVariable* customFloatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(customPlayerVar);

				if (!defPlayerVar) {
					defPlayerVar = defaultPlayerVars.emplace_back(std::make_unique<FloatPlayerVariable>(name)).get();

					FloatPlayerVariable* defFloatPlayerVar = reinterpret_cast<FloatPlayerVariable*>(defPlayerVar);
					defFloatPlayerVar->SetValues(floatPlayerVar->value);
				}

				floatPlayerVar->SetValues(value);
				customFloatPlayerVar->SetValues(value);
			} else if constexpr (std::is_same_v<T, bool>) {
				if (playerVar->GetType() != PlayerVarType::Bool)
					return;
				if (!customPlayerVar)
					customPlayerVar = customPlayerVars.emplace_back(std::make_unique<BoolPlayerVariable>(name)).get();
				
				BoolPlayerVariable* boolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(playerVar);
				BoolPlayerVariable* customBoolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(customPlayerVar);

				if (!defPlayerVar) {
					defPlayerVar = defaultPlayerVars.emplace_back(std::make_unique<BoolPlayerVariable>(name)).get();

					BoolPlayerVariable* defBoolPlayerVar = reinterpret_cast<BoolPlayerVariable*>(defPlayerVar);
					defBoolPlayerVar->SetValues(boolPlayerVar->value);
				}

				boolPlayerVar->SetValues(value);
				customBoolPlayerVar->SetValues(value);
			}
		}

		template <typename T>
		static void ManagePlayerVarByBool(const std::string& name, const T valueIfTrue, const T valueIfFalse, bool boolVal, bool usePreviousVal = true) {
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
		static bool IsPlayerVarManagedByBool(const std::string& name) {
			if (!gotPlayerVars)
				return false;

			return prevBoolValueMap.find(name) != prevBoolValueMap.end() && !prevBoolValueMap[name];
		}

		static PlayerVariables* Get();
	};
}