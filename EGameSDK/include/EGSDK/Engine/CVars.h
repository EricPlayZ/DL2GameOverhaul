#pragma once
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Engine\VarBase.h>
#include <EGSDK\Engine\VarMapBase.h>
#include <EGSDK\Engine\VarManagerBase.h>

namespace EGSDK::Engine {
	class EGameSDK_API CVar : public VarBase {
	public:
		union {
			ClassHelpers::StaticBuffer<0x50, uint32_t> valueOffset;
		};

		explicit CVar(const std::string& name);
		explicit CVar(const std::string& name, VarType type);

		VarValueType& GetValue();
		void SetValue(const VarValueType& value);
		void AddValuePtr(uint64_t* ptr);
	private:
		struct VarValue {
			VarValueType value;
			std::vector<uint64_t*> valuePtrs;
		};
		static std::unordered_map<const CVar*, VarValue> varValues;
	};

	class EGameSDK_API FloatCVar : public CVar {
	public:
		explicit FloatCVar(const std::string& name);
	};
	class EGameSDK_API IntCVar : public CVar {
	public:
		explicit IntCVar(const std::string& name);
	};
	class EGameSDK_API Vec3CVar : public CVar {
	public:
		explicit Vec3CVar(const std::string& name);
	};
	class EGameSDK_API Vec4CVar : public CVar {
	public:
		explicit Vec4CVar(const std::string& name);
	};

	class EGameSDK_API CVarMap : public VarMapBase<CVar> {
	public:
		using VarMapBase<CVar>::Find;
		using VarMapBase<CVar>::none_of;

		bool none_of(uint32_t valueOffset);

		CVar* Find(uint32_t valueOffset) const;

		std::unique_ptr<CVar>& try_emplace(std::unique_ptr<CVar> var) override;

		void Erase(const std::string& name) override;
	private:
		std::unordered_map<uint32_t, CVar*> varsByValueOffset;
	};

	class EGameSDK_API CVars : public VarManagerBase<CVarMap, CVar> {};
}