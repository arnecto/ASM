#include "Config.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <shlobj.h>
#include <codecvt>
#include <locale>

using json = nlohmann::json;

Config::Config() {}

std::string Config::wstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
}

std::wstring Config::utf8ToWstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}

std::wstring Config::getConfigFilePath() const {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        std::wstring configPath = path;
        configPath += L"\\AudioSwitchManager";
        CreateDirectoryW(configPath.c_str(), nullptr);
        configPath += L"\\config.json";
        return configPath;
    }
    return L"config.json";
}

bool Config::load(const std::wstring& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    try {
        json j;
        file >> j;

        m_processVolumes.clear();
        if (j.contains("processVolumes") && j["processVolumes"].is_array()) {
            for (const auto& item : j["processVolumes"]) {
                ProcessVolumeConfig config;
                config.processName = utf8ToWstring(item["processName"].get<std::string>());
                config.volume = item["volume"].get<float>();
                m_processVolumes.push_back(config);
            }
        }

        m_hotkeys.clear();
        if (j.contains("hotkeys") && j["hotkeys"].is_array()) {
            for (const auto& item : j["hotkeys"]) {
                HotkeyBindingConfig config;
                config.id = item["id"].get<int>();
                config.modifiers = item["modifiers"].get<UINT>();
                config.vk = item["vk"].get<UINT>();
                config.action = utf8ToWstring(item["action"].get<std::string>());
                config.description = utf8ToWstring(item["description"].get<std::string>());
                m_hotkeys.push_back(config);
            }
        }

        m_deviceProfiles.clear();
        if (j.contains("deviceProfiles") && j["deviceProfiles"].is_array()) {
            for (const auto& item : j["deviceProfiles"]) {
                DeviceProfileConfig config;
                config.name = utf8ToWstring(item["name"].get<std::string>());
                config.playbackDeviceId = utf8ToWstring(item["playbackDeviceId"].get<std::string>());
                config.captureDeviceId = utf8ToWstring(item["captureDeviceId"].get<std::string>());
                m_deviceProfiles.push_back(config);
            }
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool Config::save(const std::wstring& filePath) {
    json j;

    json processVolumesArray = json::array();
    for (const auto& config : m_processVolumes) {
        json item;
        item["processName"] = wstringToUtf8(config.processName);
        item["volume"] = config.volume;
        processVolumesArray.push_back(item);
    }
    j["processVolumes"] = processVolumesArray;

    json hotkeysArray = json::array();
    for (const auto& config : m_hotkeys) {
        json item;
        item["id"] = config.id;
        item["modifiers"] = config.modifiers;
        item["vk"] = config.vk;
        item["action"] = wstringToUtf8(config.action);
        item["description"] = wstringToUtf8(config.description);
        hotkeysArray.push_back(item);
    }
    j["hotkeys"] = hotkeysArray;

    json deviceProfilesArray = json::array();
    for (const auto& config : m_deviceProfiles) {
        json item;
        item["name"] = wstringToUtf8(config.name);
        item["playbackDeviceId"] = wstringToUtf8(config.playbackDeviceId);
        item["captureDeviceId"] = wstringToUtf8(config.captureDeviceId);
        deviceProfilesArray.push_back(item);
    }
    j["deviceProfiles"] = deviceProfilesArray;

    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << j.dump(4);
    return true;
}

void Config::addProcessVolume(const ProcessVolumeConfig& config) {
    removeProcessVolume(config.processName);
    m_processVolumes.push_back(config);
}

void Config::removeProcessVolume(const std::wstring& processName) {
    m_processVolumes.erase(
        std::remove_if(m_processVolumes.begin(), m_processVolumes.end(),
            [&processName](const ProcessVolumeConfig& c) { return c.processName == processName; }),
        m_processVolumes.end()
    );
}

void Config::addHotkey(const HotkeyBindingConfig& config) {
    removeHotkey(config.id);
    m_hotkeys.push_back(config);
}

void Config::removeHotkey(int id) {
    m_hotkeys.erase(
        std::remove_if(m_hotkeys.begin(), m_hotkeys.end(),
            [id](const HotkeyBindingConfig& c) { return c.id == id; }),
        m_hotkeys.end()
    );
}

void Config::addDeviceProfile(const DeviceProfileConfig& config) {
    removeDeviceProfile(config.name);
    m_deviceProfiles.push_back(config);
}

void Config::removeDeviceProfile(const std::wstring& name) {
    m_deviceProfiles.erase(
        std::remove_if(m_deviceProfiles.begin(), m_deviceProfiles.end(),
            [&name](const DeviceProfileConfig& c) { return c.name == name; }),
        m_deviceProfiles.end()
    );
}
