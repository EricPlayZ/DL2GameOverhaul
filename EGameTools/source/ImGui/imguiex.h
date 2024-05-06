#pragma once
#include "..\core.h"

namespace ImGui {
	extern void StyleScaleAllSizes(ImGuiStyle* style, const float scale_factor, ImGuiStyle* defStyle = nullptr);
	extern void SpanNextTabAcrossWidth(const float width, const size_t tabs = 1);
	extern void EndTabBarEx();
	extern bool Button(const char* label, const char* tooltip, const ImVec2& size_arg = ImVec2(0, 0));
	extern bool Checkbox(const char* label, bool* v, const char* tooltip);
	extern bool Checkbox(const char* label, Option* v);
	extern bool Checkbox(const char* label, Option* v, const char* tooltip);
	extern bool CheckboxHotkey(const char* label, KeyBindOption* v, const char* tooltip = nullptr);
	extern void TextCentered(const char* text, const bool calculateWithScrollbar = true);
	extern void TextCenteredColored(const char* text, const ImU32 col, const bool calculateWithScrollbar = true);
	extern bool ButtonCentered(const char* label, const ImVec2 size = ImVec2(0.0f, 0.0f));
	extern void SeparatorTextColored(const char* text, const ImU32 col);
	extern void Spacing(const ImVec2 size, const bool customPosOffset = false);
}