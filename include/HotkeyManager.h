#pragma once

#include <Windows.h>
#include <string>
#include <map>
#include <functional>

struct HotkeyConfig {
    UINT modifiers;
    UINT vk;
    std::wstring description;
    std::function<void()> callback;
};

class HotkeyManager {
public:
    HotkeyManager();
    ~HotkeyManager();

    bool registerHotkey(int id, UINT modifiers, UINT vk, const std::wstring& description, std::function<void()> callback);
    bool unregisterHotkey(int id);
    void unregisterAll();

    bool processHotkey(int id);

    std::map<int, HotkeyConfig> getRegisteredHotkeys() const { return m_hotkeys; }

private:
    std::map<int, HotkeyConfig> m_hotkeys;
};
