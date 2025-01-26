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
			EGSDK::ClassHelpers::StaticBuffer<0x8, const char*> value; // remove 0x2 bit to access ptr
			EGSDK::ClassHelpers::StaticBuffer<0x10, const char*> defaultValue; // remove 0x2 bit to access ptr
		};
		explicit StringPlayerVariable(const std::string& name);

		const char* GetValue();
		const char* GetDefaultValue();
		void SetValues(const std::string& value);
	};
	class EGameSDK_API FloatPlayerVariable : public PlayerVariable {
	public:
		union {
			EGSDK::ClassHelpers::StaticBuffer<0x8, float> value;
			EGSDK::ClassHelpers::StaticBuffer<0xC, float> defaultValue;
		};
		explicit FloatPlayerVariable(const std::string& name);

		float GetValue();
		float GetDefaultValue();
		void SetValues(float value);
	};
	class EGameSDK_API BoolPlayerVariable : public PlayerVariable {
	public:
		union {
			EGSDK::ClassHelpers::StaticBuffer<0x8, bool> value;
			EGSDK::ClassHelpers::StaticBuffer<0x9, bool> defaultValue;
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
		void reserve(size_t count);
		size_t size();

		std::unique_ptr<PlayerVariable>* FindPtr(const std::string& name);
		PlayerVariable* Find(const std::string& name);
		bool Erase(const std::string& name);

		template <typename Callable, typename... Args>
		void ForEach(Callable&& func, Args&&... args) {
			std::lock_guard<std::mutex> lock(mutex);
			for (const auto& name : playerVarsOrder)
				func(playerVars.at(name), std::forward<Args>(args)...);
		}
	private:
		std::unordered_map<std::string, std::unique_ptr<PlayerVariable>> playerVars{};
		std::vector<std::string> playerVarsOrder{};
		mutable std::mutex mutex{};
	};

	class EGameSDK_API PlayerVariables {
	public:
		static PlayerVarMap playerVars;
		static PlayerVarMap customPlayerVars;
		static PlayerVarMap defaultPlayerVars;
		static PlayerVarMap customDefaultPlayerVars;
		static std::atomic<bool> gotPlayerVars;

#ifdef EGameSDK_EXPORTS
		static std::unordered_map<std::string, std::any> prevPlayerVarValueMap;
		static std::unordered_map<std::string, bool> prevBoolValueMap;
		static std::unordered_map<std::string, uint64_t> playerVarOwnerMap;

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
	private:
		static std::mutex mutex;
	};
}