#pragma once
#include <string>
#include <any>
#include <mutex>
#include <unordered_map>
#include <EGSDK\Exports.h>

namespace EGSDK::Engine {
    template <typename VarMapType, typename VarType>
    class EGameSDK_API VarManagerBase {
    public:
        static VarMapType vars;
        static VarMapType customVars;
        static VarMapType defaultVars;

#ifdef EGameSDK_EXPORTS
        static std::unordered_map<std::string, std::any> prevVarValueMap;
        static std::unordered_map<std::string, bool> prevBoolValueMap;
        static std::unordered_map<std::string, uint64_t> varOwnerMap;
#endif

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

        template <typename T>
        static void ManageVarByBool(const std::string& name, T valueIfTrue, T valueIfFalse, bool boolVal, bool usePreviousVal = true);
        static bool IsVarManagedByBool(const std::string& name);
    private:
        static std::recursive_mutex mutex;
    };
}