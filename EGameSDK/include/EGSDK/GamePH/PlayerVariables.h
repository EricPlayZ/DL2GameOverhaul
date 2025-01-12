#pragma once
#include <any>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
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
		static std::mutex mutex;
	};

	class EGameSDK_API StringPlayerVariable : public PlayerVariable {
	public:
		union {
			EGSDK::ClassHelpers::buffer<0x8, const char*> value; // remove 0x2 bit to access ptr
			EGSDK::ClassHelpers::buffer<0x10, const char*> defaultValue; // remove 0x2 bit to access ptr
		};
		explicit StringPlayerVariable(const std::string& name);

		const char* GetValue();
		const char* GetDefaultValue();
		void SetValues(const std::string& value);
	};
	class EGameSDK_API FloatPlayerVariable : public PlayerVariable {
	public:
		union {
			EGSDK::ClassHelpers::buffer<0x8, float> value;
			EGSDK::ClassHelpers::buffer<0xC, float> defaultValue;
		};
		explicit FloatPlayerVariable(const std::string& name);

		float GetValue();
		float GetDefaultValue();
		void SetValues(float value);
	};
	class EGameSDK_API BoolPlayerVariable : public PlayerVariable {
	public:
		union {
			EGSDK::ClassHelpers::buffer<0x8, bool> value;
			EGSDK::ClassHelpers::buffer<0x9, bool> defaultValue;
		};
		explicit BoolPlayerVariable(const std::string& name);

		bool GetValue();
		bool GetDefaultValue();
		void SetValues(bool value);
	};

	class EGameSDK_API PlayerVarMap {
	public:
		PlayerVarMap() = default;
		PlayerVarMap(const PlayerVarMap&) = delete;
		PlayerVarMap& operator=(const PlayerVarMap&) = delete;

		PlayerVarMap(PlayerVarMap&&) noexcept = default;
		PlayerVarMap& operator=(PlayerVarMap&&) noexcept = default;

		std::unique_ptr<PlayerVariable>& try_emplace(std::unique_ptr<PlayerVariable> playerVar);
		bool empty();
		bool none_of(const std::string& name);

		std::unique_ptr<PlayerVariable>* FindPtr(const std::string& name);
		PlayerVariable* Find(const std::string& name);
		bool Erase(const std::string& name);

		template <typename Callable, typename... Args>
		void ForEach(Callable&& func, Args&&... args) {
			std::lock_guard<std::mutex> lock(_mutex);
			for (const auto& name : _order)
				func(_playerVars.at(name), std::forward<Args>(args)...);
		}
	private:
		std::unordered_map<std::string, std::unique_ptr<PlayerVariable>> _playerVars{};
		std::vector<std::string> _order{};
		mutable std::mutex _mutex{};
	};

	class EGameSDK_API PlayerVariables {
	public:
		static PlayerVarMap playerVars;
		static PlayerVarMap customPlayerVars;
		static PlayerVarMap defaultPlayerVars;
		static PlayerVarMap customDefaultPlayerVars;
		static std::atomic<bool> gotPlayerVars;

		static std::unordered_map<std::string, std::any> prevPlayerVarValueMap;
		static std::unordered_map<std::string, bool> prevBoolValueMap;

#ifdef EGameSDK_EXPORTS
		static void GetPlayerVars();
		static bool SortPlayerVars();
#endif

		template <typename T>
		static T GetPlayerVarValue(const std::string& name);
		template <typename T>
		static void ChangePlayerVar(const std::string& name, const T value, PlayerVariable* playerVar = nullptr);
		template <typename T>
		static void ChangePlayerVarFromList(const std::string& name, const T value, PlayerVariable* playerVar = nullptr);

		template <typename T>
		static void ManagePlayerVarByBool(const std::string& name, const T valueIfTrue, const T valueIfFalse, bool boolVal, bool usePreviousVal = true);
		static bool IsPlayerVarManagedByBool(const std::string& name);

		static PlayerVariables* Get();
	};
}