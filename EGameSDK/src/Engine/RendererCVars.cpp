#include <algorithm>
#include <spdlog\spdlog.h>
#include <EGSDK\Offsets.h>
#include <EGSDK\Utils\Time.h>
#include <EGSDK\Engine\RendererCVars.h>

namespace EGSDK::Engine {
	RendererCVar::RendererCVar(const std::string& name) {
		this->name = name;
		this->type = RendererCVarType::NONE;
	}
	RendererCVar::RendererCVar(const std::string& name, DWORD64 address) {
		this->name = name;
		this->type = RendererCVarType::NONE;
		this->address = address;
	}
	const char* RendererCVar::GetName() {
		return name.c_str();
	}
	void RendererCVar::SetName(const std::string& name) {
		this->name = name;
	}
	RendererCVarType RendererCVar::GetType() {
		return type;
	}
	void RendererCVar::SetType(RendererCVarType type) {
		this->type = type;
	}
	DWORD64 RendererCVar::GetAddress() {
		return address;
	}
	void RendererCVar::SetAddress(DWORD64 address) {
		this->address = address;
	}

	FloatRendererCVar::FloatRendererCVar(const std::string& name) : RendererCVar(name) {
		SetType(RendererCVarType::Float);
	}
	FloatRendererCVar::FloatRendererCVar(const std::string& name, DWORD64 address) : RendererCVar(name, address) {
		SetType(RendererCVarType::Float);
	}
	float FloatRendererCVar::GetValue() {
		return value;
	}
	void FloatRendererCVar::SetValue(float value) {
		this->value = value;
	}

	std::unique_ptr<RendererCVar>& RendererCVarMap::try_emplace(std::unique_ptr<RendererCVar> rendererCVar) {
		std::lock_guard<std::mutex> lock(mutex);
		const std::string& name = rendererCVar->GetName();
		auto [it, inserted] = _rendererCVars.try_emplace(name, std::move(rendererCVar));
		if (inserted) {
			_order.emplace_back(name);
			_rendererCVarsByAddress[it->second->GetAddress()] = it->second.get();
		}
		return it->second;
	}
	bool RendererCVarMap::empty() {
		std::lock_guard<std::mutex> lock(mutex);
		return _rendererCVars.empty();
	}
	bool RendererCVarMap::none_of(const std::string& name) {
		std::lock_guard<std::mutex> lock(mutex);
		return _rendererCVars.find(name) == _rendererCVars.end();
	}
	bool RendererCVarMap::none_of(DWORD64 address) {
		std::lock_guard<std::mutex> lock(mutex);
		return _rendererCVarsByAddress.find(address) == _rendererCVarsByAddress.end();
	}

	std::unique_ptr<RendererCVar>* RendererCVarMap::FindPtr(const std::string& name) {
		std::lock_guard<std::mutex> lock(mutex);
		auto it = _rendererCVars.find(name);
		return it == _rendererCVars.end() ? nullptr : &it->second;
	}
	RendererCVar* RendererCVarMap::Find(const std::string& name) {
		std::lock_guard<std::mutex> lock(mutex);
		auto it = _rendererCVars.find(name);
		return it == _rendererCVars.end() ? nullptr : it->second.get();
	}
	RendererCVar* RendererCVarMap::Find(DWORD64 address) {
		std::lock_guard<std::mutex> lock(mutex);
		auto it = _rendererCVarsByAddress.find(address);
		return it == _rendererCVarsByAddress.end() ? nullptr : it->second;
	}
	bool RendererCVarMap::Erase(const std::string& name) {
		std::lock_guard<std::mutex> lock(mutex);
		auto it = _rendererCVars.find(name);
		if (it != _rendererCVars.end())
			return _rendererCVarsByAddress.erase(it->second->GetAddress()) > 0 && _rendererCVars.erase(name) > 0;
		return false;
	}

	RendererCVarMap RendererCVars::rendererCVars{};
	RendererCVarMap RendererCVars::customRendererCVars{};
	RendererCVarMap RendererCVars::defaultRendererCVars{};

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
	float RendererCVars::GetRendererCVarValue(const std::string& name) {
		auto rendererCVar = rendererCVars.Find(name);
		if (!rendererCVar)
			return getDefaultValue<float>();

		return reinterpret_cast<FloatRendererCVar*>(rendererCVar)->GetValue();
	}
	void RendererCVars::ChangeRendererCVar(const std::string& name, float value, RendererCVar* rendererCVar) {
		if (!rendererCVar)
			rendererCVar = rendererCVars.Find(name);
		if (!rendererCVar)
			return;

		if (rendererCVar->GetType() == RendererCVarType::Float)
			reinterpret_cast<FloatRendererCVar*>(rendererCVar)->SetValue(value);
	}
	void RendererCVars::ChangeRendererCVarFromList(const std::string& name, float value, RendererCVar* rendererCVar) {
		if (!rendererCVar)
			rendererCVar = rendererCVars.Find(name);
		if (!rendererCVar)
			return;

		auto customRendererCVar = customRendererCVars.Find(name);
		auto defRendererCVar = defaultRendererCVars.Find(name);

		if (rendererCVar->GetType() != RendererCVarType::Float)
			return;
		if (!customRendererCVar)
			customRendererCVar = customRendererCVars.try_emplace(std::make_unique<FloatRendererCVar>(name, rendererCVar->GetAddress())).get();
		if (!defRendererCVar) {
			defRendererCVar = defaultRendererCVars.try_emplace(std::make_unique<FloatRendererCVar>(name)).get();
			reinterpret_cast<FloatRendererCVar*>(defRendererCVar)->SetValue(reinterpret_cast<FloatRendererCVar*>(rendererCVar)->GetValue());
		}

		reinterpret_cast<FloatRendererCVar*>(rendererCVar)->SetValue(value);
		reinterpret_cast<FloatRendererCVar*>(customRendererCVar)->SetValue(value);
	}
}