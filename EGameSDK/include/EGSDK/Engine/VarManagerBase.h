#pragma once
#include <string>
#include <EGSDK\Exports.h>

namespace EGSDK::Engine {
    template <typename VarMapType, typename VarType>
    class EGameSDK_API VarManagerBase {
    public:
        static VarMapType vars;
        static VarMapType customVars;
        static VarMapType defaultVars;

        template <typename T>
        static T* GetVarValue(const std::string& name);
        template <typename T>
        static T* GetVarValue(VarType* var);
        template <typename T>
        static T* GetVarValueFromMap(const std::string& name, const VarMapType& map);

        template <typename T>
        static void ChangeVar(const std::string& name, T value);
        template <typename T>
        static void ChangeVar(VarType* var, T value);
        template <typename T>
        static void ChangeVarFromList(const std::string& name, T value);
        template <typename T>
        static void ChangeVarFromList(VarType* var, T value);
    };
}