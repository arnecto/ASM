#include "HotkeyManager.h"

HotkeyManager::HotkeyManager() {}

HotkeyManager::~HotkeyManager() {
    unregisterAll();
}

bool HotkeyManager::registerHotkey(int id, UINT modifiers, UINT vk, const std::wstring& description, std::function<void()> callback) {
    if (m_hotkeys.find(id) != m_hotkeys.end()) {
        unregisterHotkey(id);
    }

    if (!RegisterHotKey(nullptr, id, modifiers, vk)) {
        return false;
    }

    HotkeyConfig config;
    config.modifiers = modifiers;
    config.vk = vk;
    config.description = description;
    config.callback = callback;

    m_hotkeys[id] = config;
    return true;
}

bool HotkeyManager::unregisterHotkey(int id) {
    auto it = m_hotkeys.find(id);
    if (it == m_hotkeys.end()) {
        return false;
    }

    UnregisterHotKey(nullptr, id);
    m_hotkeys.erase(it);
    return true;
}

void HotkeyManager::unregisterAll() {
    for (const auto& pair : m_hotkeys) {
        UnregisterHotKey(nullptr, pair.first);
    }
    m_hotkeys.clear();
}

bool HotkeyManager::processHotkey(int id) {
    auto it = m_hotkeys.find(id);
    if (it != m_hotkeys.end() && it->second.callback) {
        it->second.callback();
        return true;
    }
    return false;
}
