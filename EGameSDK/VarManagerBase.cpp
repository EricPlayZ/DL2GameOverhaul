#include <EGSDK\Vec4.h>
#include <EGSDK\Engine\VarManagerBase.h>
#include <EGSDK\Engine\VarMapBase.h>
#include <EGSDK\Engine\CVars.h>

namespace EGSDK::Engine {
#define StaticAssertValueType static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, float> || std::is_same_v<T, int> || std::is_same_v<T, Vec3> || std::is_same_v<T, Vec4> || std::is_same_v<T, bool>, "Invalid type: value must be string, float, int, Vec3, Vec4 or bool")

    template <typename VarMapType, typename VarType>
    VarMapType VarManagerBase<VarMapType, VarType>::vars;
    template <typename VarMapType, typename VarType>
    VarMapType VarManagerBase<VarMapType, VarType>::customVars;
    template <typename VarMapType, typename VarType>
    VarMapType VarManagerBase<VarMapType, VarType>::defaultVars;

    template <typename VarMapType, typename VarType>
    template <typename T>
    T* VarManagerBase<VarMapType, VarType>::GetVarValue(const std::string& name) {
        StaticAssertValueType;
        auto var = vars.Find(name);
        return var ? std::get_if<T>(&var->GetValue()) : nullptr;
    }
    template <typename VarMapType, typename VarType>
    template <typename T>
    T* VarManagerBase<VarMapType, VarType>::GetVarValue(VarType* var) {
        StaticAssertValueType;
        return var ? std::get_if<T>(&var->GetValue()) : nullptr;
    }
    template <typename VarMapType, typename VarType>
    template <typename T>
    T* VarManagerBase<VarMapType, VarType>::GetVarValueFromMap(const std::string& name, const VarMapType& map) {
        StaticAssertValueType;
        auto var = map.Find(name);
        return var ? std::get_if<T>(&var->GetValue()) : nullptr;
    }

    template <typename VarMapType, typename VarType>
    template <typename T>
    void VarManagerBase<VarMapType, VarType>::ChangeVar(const std::string& name, T value) {
        StaticAssertValueType;
        auto var = vars.Find(name);
        if (var)
            var->SetValue(value);
    }
    template <typename VarMapType, typename VarType>
    template <typename T>
    void VarManagerBase<VarMapType, VarType>::ChangeVar(VarType* var, T value) {
        StaticAssertValueType;
        if (var)
            var->SetValue(value);
    }
    template <typename VarMapType, typename VarType>
    template <typename T>
    void VarManagerBase<VarMapType, VarType>::ChangeVarFromList(const std::string& name, T value) {
        StaticAssertValueType;

        auto var = vars.Find(name);
        if (!var)
            return;

        ChangeVarFromList<T>(var, value);
    }
    template <typename VarMapType, typename VarType>
    template <typename T>
    void VarManagerBase<VarMapType, VarType>::ChangeVarFromList(VarType* var, T value) {
        StaticAssertValueType;

        if (!var)
            return;

        auto customVar = customVars.Find(var->GetName());
        auto defVar = defaultVars.Find(var->GetName());

        if (!customVar)
            customVar = customVars.try_emplace(std::make_unique<VarType>(var->GetName(), var->GetType())).get();
        if (!defVar) {
            defVar = defaultVars.try_emplace(std::make_unique<VarType>(var->GetName(), var->GetType())).get();
            if (auto varValue = GetVarValue<T>(var); defVar && varValue)
                defVar->SetValue(*varValue);
        }

        if (customVar)
            customVar->SetValue(value);
        var->SetValue(value);
    }

    template EGameSDK_API class VarManagerBase<CVarMap, CVar>;

    template EGameSDK_API float* VarManagerBase<CVarMap, CVar>::GetVarValue<float>(const std::string&);
    template EGameSDK_API float* VarManagerBase<CVarMap, CVar>::GetVarValue<float>(CVar*);
    template EGameSDK_API int* VarManagerBase<CVarMap, CVar>::GetVarValue<int>(const std::string&);
    template EGameSDK_API int* VarManagerBase<CVarMap, CVar>::GetVarValue<int>(CVar*);
    template EGameSDK_API float* VarManagerBase<CVarMap, CVar>::GetVarValueFromMap<float>(const std::string&, const CVarMap& map);
    template EGameSDK_API int* VarManagerBase<CVarMap, CVar>::GetVarValueFromMap<int>(const std::string&, const CVarMap& map);

    template EGameSDK_API void VarManagerBase<CVarMap, CVar>::ChangeVar<float>(const std::string&, float);
    template EGameSDK_API void VarManagerBase<CVarMap, CVar>::ChangeVar<float>(CVar*, float);
    template EGameSDK_API void VarManagerBase<CVarMap, CVar>::ChangeVar<int>(const std::string&, int);
    template EGameSDK_API void VarManagerBase<CVarMap, CVar>::ChangeVar<int>(CVar*, int);
    template EGameSDK_API void VarManagerBase<CVarMap, CVar>::ChangeVarFromList<float>(const std::string&, float);
    template EGameSDK_API void VarManagerBase<CVarMap, CVar>::ChangeVarFromList<float>(CVar*, float);
    template EGameSDK_API void VarManagerBase<CVarMap, CVar>::ChangeVarFromList<int>(const std::string&, int);
    template EGameSDK_API void VarManagerBase<CVarMap, CVar>::ChangeVarFromList<int>(CVar*, int);
}