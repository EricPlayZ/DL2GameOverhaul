#include <unordered_map>
#include <string_view>
#define _USE_MATH_DEFINES
#include <cmath>
#include <imgui_internal.h>

namespace ImGui {
    std::unordered_map<std::string_view, float> AnimValueStack{};

    const float AnimEaseInSine(const float blendPoint) { return 1.0f - std::cos((blendPoint * static_cast<float>(M_PI)) / 2.0f); }
    const float AnimEaseOutSine(const float blendPoint) { return std::sin((blendPoint * static_cast<float>(M_PI)) / 2.0f); }
    const float AnimEaseInOutSine(const float blendPoint) { return -(std::cos(static_cast<float>(M_PI) * blendPoint) - 1.0f) / 2.0f; }

    static const float SetAnimTime(const std::string_view& valueName, const float animSpeed, const bool resetBlendPoint = false, const float (*easingFunction)(float) = nullptr) {
        const ImGuiContext& g = *GImGui;

        float blendPoint = 0.0f;
        if (!resetBlendPoint)
            blendPoint = AnimValueStack[valueName];

        if (blendPoint == 1.0f)
            return blendPoint;

        blendPoint += g.IO.DeltaTime / animSpeed;
        blendPoint = ImMin(blendPoint, 1.0f);

        AnimValueStack[valueName] = blendPoint;

        if (easingFunction)
            blendPoint = easingFunction(blendPoint);

        return blendPoint;
    }
    const float AnimateLerp(const std::string_view& valueName, const float a, const float b, const float animSpeed, const bool resetBlendPoint, const float (*easingFunction)(float)) {
        const float blendPoint = SetAnimTime(valueName, animSpeed, resetBlendPoint, easingFunction);
        return ImLerp<float>(a, b, blendPoint);
    }
}