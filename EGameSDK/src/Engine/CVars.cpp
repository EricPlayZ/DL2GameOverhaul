#include <algorithm>
#include <EGSDK\Engine\CVars.h>

namespace EGSDK::Engine {
	std::unordered_map<const CVar*, CVar::VarValue> CVar::varValues;
	CVar::CVar(const std::string& name) : VarBase(name) {}
	CVar::CVar(const std::string& name, VarType type) : VarBase(name, type) {}
	VarValueType& CVar::GetValue() {
		std::lock_guard<decltype(mutex)> lock(mutex);
		auto it = varValues.find(this);
		switch (GetType()) {
			case VarType::Float:
				if (it == varValues.end())
					return varValues[this].value = 0.0f;
				if (!it->second.valuePtrs.size())
					return it->second.value;
				if (!it->second.valuePtrs[0])
					return it->second.value;

				it->second.value = *reinterpret_cast<float*>(it->second.valuePtrs[0]);
				return it->second.value;
			case VarType::Int:
				if (it == varValues.end())
					return varValues[this].value = 0;
				if (!it->second.valuePtrs.size())
					return it->second.value;
				if (!it->second.valuePtrs[0])
					return it->second.value;

				it->second.value = *reinterpret_cast<int*>(it->second.valuePtrs[0]);
				return it->second.value;
			case VarType::Vec3:
				if (it == varValues.end())
					return varValues[this].value = Vec3();
				if (!it->second.valuePtrs.size())
					return it->second.value;
				if (!it->second.valuePtrs[0])
					return it->second.value;

				it->second.value = *reinterpret_cast<Vec3*>(it->second.valuePtrs[0]);
				return it->second.value;
			case VarType::Vec4:
				if (it == varValues.end())
					return varValues[this].value = Vec4();
				if (!it->second.valuePtrs.size())
					return it->second.value;
				if (!it->second.valuePtrs[0])
					return it->second.value;

				it->second.value = *reinterpret_cast<Vec4*>(it->second.valuePtrs[0]);
				return it->second.value;
			default:
				if (it == varValues.end())
					return varValues[this].value = 0;
				return it->second.value;
		}
	}
	void CVar::SetValue(const VarValueType& value) {
		std::lock_guard<decltype(mutex)> lock(mutex);
		switch (GetType()) {
			case VarType::Float:
			{
				if (auto actualValue = std::get_if<float>(&value)) {
					for (const auto& ptr : varValues[this].valuePtrs) {
						if (ptr)
							*reinterpret_cast<float*>(ptr) = *actualValue;
					}
					varValues[this].value = *actualValue;
				}
				break;
			}
			case VarType::Int:
			{
				if (auto actualValue = std::get_if<int>(&value)) {
					for (const auto& ptr : varValues[this].valuePtrs) {
						if (ptr)
							*reinterpret_cast<int*>(ptr) = *actualValue;
					}
					varValues[this].value = *actualValue;
				}
				break;
			}
			case VarType::Vec3:
			{
				if (auto actualValue = std::get_if<Vec3>(&value)) {
					for (const auto& ptr : varValues[this].valuePtrs) {
						if (ptr)
							*reinterpret_cast<Vec3*>(ptr) = *actualValue;
					}
					varValues[this].value = *actualValue;
				}
				break;
			}
			case VarType::Vec4:
			{
				if (auto actualValue = std::get_if<Vec4>(&value)) {
					for (const auto& ptr : varValues[this].valuePtrs) {
						if (ptr)
							*reinterpret_cast<Vec4*>(ptr) = *actualValue;
					}
					varValues[this].value = *actualValue;
				}
				break;
			}
		}
	}
	void CVar::AddValuePtr(uint64_t* ptr) {
		std::lock_guard<decltype(mutex)> lock(mutex);
		varValues[this].valuePtrs.push_back(ptr);
	}

	FloatCVar::FloatCVar(const std::string& name) : CVar(name) {
		SetType(VarType::Float);
	}
	IntCVar::IntCVar(const std::string& name) : CVar(name) {
		SetType(VarType::Int);
	}
	Vec3CVar::Vec3CVar(const std::string& name) : CVar(name) {
		SetType(VarType::Vec3);
	}
	Vec4CVar::Vec4CVar(const std::string& name) : CVar(name) {
		SetType(VarType::Vec4);
	}

	std::unique_ptr<CVar>& CVarMap::try_emplace(std::unique_ptr<CVar> cVar) {
		std::lock_guard<decltype(mutex)> lock(mutex);
		const std::string& name = cVar->GetName();
		auto [it, inserted] = vars.try_emplace(name, std::move(cVar));
		if (inserted) {
			varsOrdered.emplace_back(name);
			if (uint32_t valueOffset = it->second->valueOffset.data; valueOffset || valueOffset != 0xCDCDCDCD)
				varsByValueOffset[valueOffset] = it->second.get();
		}
		return it->second;
	}
	bool CVarMap::none_of(uint32_t valueOffset) {
		std::lock_guard<decltype(mutex)> lock(mutex);
		return varsByValueOffset.find(valueOffset) == varsByValueOffset.end();
	}

	CVar* CVarMap::Find(uint32_t valueOffset) const {
		std::lock_guard<decltype(mutex)> lock(mutex);
		auto it = varsByValueOffset.find(valueOffset);
		return it == varsByValueOffset.end() ? nullptr : it->second;
	}
	void CVarMap::Erase(const std::string& name) {
		std::lock_guard<decltype(mutex)> lock(mutex);
		auto it = vars.find(name);
		if (it == vars.end())
			return;

		auto orderIt = std::find(varsOrdered.begin(), varsOrdered.end(), name);
		if (orderIt != varsOrdered.end())
			varsOrdered.erase(orderIt);

		varsByValueOffset.erase(it->second->valueOffset.data);
		vars.erase(it);
	}
}