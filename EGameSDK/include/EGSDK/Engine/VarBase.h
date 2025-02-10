#pragma once
#include <unordered_map>
#include <string>
#include <mutex>
#include <variant>
#include <EGSDK\Exports.h>
#include <EGSDK\Vec3.h>
#include <EGSDK\Vec4.h>

namespace EGSDK::Engine {
    EGameSDK_API enum class VarType {
        NONE = 0,
        String,
        Float,
        Int,
        Vec3,
        Vec4,
        Bool
    };

    using VarValueType = std::variant<std::string, float, int, Vec3, Vec4, bool>;

    class EGameSDK_API VarBase {
    public:
        VarBase(const std::string& name);
        VarBase(const std::string& name, VarType type);
        ~VarBase();

        const char* GetName() const;
        void SetName(const std::string& name);

        VarType GetType() const;
        void SetType(VarType type);
    protected:
        static std::recursive_mutex mutex;
    private:
        static std::unordered_map<const VarBase*, std::string> varNames;
        static std::unordered_map<const VarBase*, VarType> varTypes;
    };
}