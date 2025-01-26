#pragma once
#include <any>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\Values.h>

namespace EGSDK::Engine {
	enum EGameSDK_API RendererCVarType {
		NONE = 0,
		Float
	};

	class EGameSDK_API RendererCVar {
	public:
		RendererCVar(const std::string& name);
		RendererCVar(const std::string& name, DWORD64 address);

		const char* GetName();
		void SetName(const std::string& name);

		RendererCVarType GetType();
		void SetType(RendererCVarType type);

		DWORD64 GetAddress();
		void SetAddress(DWORD64 address);
	private:
		std::string name{};
		RendererCVarType type = RendererCVarType::NONE;
		DWORD64 address = 0;
	};

	class EGameSDK_API FloatRendererCVar : public RendererCVar {
	public:
		explicit FloatRendererCVar(const std::string& name);
		explicit FloatRendererCVar(const std::string& name, DWORD64 address);

		float GetValue();
		void SetValue(float value);
	private:
		float value = -404.0f;
	};

	class EGameSDK_API RendererCVarMap {
	public:
		RendererCVarMap() = default;
		RendererCVarMap(const RendererCVarMap&) = delete;
		RendererCVarMap& operator=(const RendererCVarMap&) = delete;

		RendererCVarMap(RendererCVarMap&&) noexcept = default;
		RendererCVarMap& operator=(RendererCVarMap&&) noexcept = default;

		std::unique_ptr<RendererCVar>& try_emplace(std::unique_ptr<RendererCVar> rendererCVar);
		bool empty();
		bool none_of(const std::string& name);
		bool none_of(DWORD64 address);

		std::unique_ptr<RendererCVar>* FindPtr(const std::string& name);
		RendererCVar* Find(const std::string& name);
		RendererCVar* Find(DWORD64 address);
		bool Erase(const std::string& name);

		template <typename Callable, typename... Args>
		void ForEach(Callable&& func, Args&&... args) {
			std::lock_guard<std::mutex> lock(mutex);
			for (const auto& name : _order)
				func(_rendererCVars.at(name), std::forward<Args>(args)...);
		}
	private:
		std::unordered_map<std::string, std::unique_ptr<RendererCVar>> _rendererCVars{};
		std::unordered_map<DWORD64, RendererCVar*> _rendererCVarsByAddress{};
		std::vector<std::string> _order{};
		mutable std::mutex mutex{};
	};

	class EGameSDK_API RendererCVars {
	public:
		static RendererCVarMap rendererCVars;
		static RendererCVarMap customRendererCVars;
		static RendererCVarMap defaultRendererCVars;

		static float GetRendererCVarValue(const std::string& name);
		static void ChangeRendererCVar(const std::string& name, float value, RendererCVar* rendererCVar = nullptr);
		static void ChangeRendererCVarFromList(const std::string& name, float value, RendererCVar* rendererCVar = nullptr);
	};
}