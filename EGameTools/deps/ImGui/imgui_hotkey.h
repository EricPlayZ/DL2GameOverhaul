#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui\imgui.h>
#include <set>
#include <EGSDK\Utils\Time.h>

#ifndef VK_NONE
#define VK_NONE -1
#define VK_INVALID -404
#define VK_MWHEELDOWN 0x100
#define VK_MWHEELUP 0x101
#endif

namespace ImGui {
    extern bool isAnyHotkeyBtnPressed;
    extern EGSDK::Utils::Time::Timer timeSinceHotkeyBtnPressed;

    struct Key {
        Key(std::string_view name, int code, ImGuiKey imGuiCode);

        std::string_view name;
        int code;
        ImGuiKey imGuiCode;
    };
    struct VKey {
        VKey(std::string_view name, int code);

        std::string_view name;
        int code;
    };

    class Option {
    public:
        Option();
        Option(bool value);
        ~Option();
        static std::set<Option*>* GetInstances();

        bool value = false;

        void SetChangesAreDisabled(bool newValue);
        bool GetChangesAreDisabled() const;
        void Toggle();
        void Set(bool newValue);
        void SetBothValues(bool newValue);
        void SetValue(bool newValue);
        void SetPrevValue(bool newValue);
        bool GetValue() const;
        bool GetPrevValue() const;
        bool HasChanged() const;
        bool HasChangedTo(bool toValue) const;
    private:
        bool changesAreDisabled = false;
        bool previousValue = false;
    };
    class KeyBindOption : public Option {
    public:
        static bool wasAnyKeyPressed;
        static bool scrolledMouseWheelUp;
        static bool scrolledMouseWheelDown;

        KeyBindOption(int keyCode);
        ~KeyBindOption();
        static std::set<KeyBindOption*>* GetInstances();

        std::string ToStringKeyMap();
        std::string ToStringVKeyMap();
        static std::string ToStringKeyMap(int keyCode);
        static std::string ToStringVKeyMap(int keyCode);
        static int ToKeyCodeKeyMap(const std::string& keyName);
        static int ToKeyCodeVKeyMap(const std::string& keyName);
        int GetKeyBind() const;
        void ChangeKeyBind(int newKeyBind);

        bool SetToPressedKey();
    private:
        int keyCode = 0;
        static const std::array<Key, 103> keyMap;
        static const std::array<VKey, 135> virtualKeyMap;
    };

    extern void Hotkey(const std::string_view& label, KeyBindOption* key);
}