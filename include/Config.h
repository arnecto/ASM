#pragma once

#include <string>
#include <map>
#include <vector>
#include <Windows.h>

struct ProcessVolumeConfig {
    std::wstring processName;
    float volume;
};

struct HotkeyBindingConfig {
    int id;
    UINT modifiers;
    UINT vk;
    std::wstring action;
    std::wstring description;
};

struct DeviceProfileConfig {
    std::wstring name;
    std::wstring playbackDeviceId;
    std::wstring captureDeviceId;
};

class Config {
public:
    Config();

    bool load(const std::wstring& filePath);
    bool save(const std::wstring& filePath);

    std::vector<ProcessVolumeConfig> getProcessVolumes() const { return m_processVolumes; }
    std::vector<HotkeyBindingConfig> getHotkeys() const { return m_hotkeys; }
    std::vector<DeviceProfileConfig> getDeviceProfiles() const { return m_deviceProfiles; }

    void addProcessVolume(const ProcessVolumeConfig& config);
    void removeProcessVolume(const std::wstring& processName);

    void addHotkey(const HotkeyBindingConfig& config);
    void removeHotkey(int id);

    void addDeviceProfile(const DeviceProfileConfig& config);
    void removeDeviceProfile(const std::wstring& name);

    std::wstring getConfigFilePath() const;

private:
    std::vector<ProcessVolumeConfig> m_processVolumes;
    std::vector<HotkeyBindingConfig> m_hotkeys;
    std::vector<DeviceProfileConfig> m_deviceProfiles;

    std::string wstringToUtf8(const std::wstring& wstr);
    std::wstring utf8ToWstring(const std::string& str);
};
