#pragma once

#include <Windows.h>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <audiopolicy.h>
#include <mmdeviceapi.h>

struct ProcessVolumeRule {
    std::wstring processName;
    float targetVolume;
    bool enabled;
};

class ProcessVolumeManager {
public:
    ProcessVolumeManager();
    ~ProcessVolumeManager();

    bool initialize();
    void cleanup();

    void addRule(const std::wstring& processName, float volume);
    void removeRule(const std::wstring& processName);
    void updateRule(const std::wstring& processName, float volume);

    std::vector<ProcessVolumeRule> getRules() const;

    void applyRules();

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    std::map<std::wstring, ProcessVolumeRule> m_rules;
    bool m_enabled;
    IMMDeviceEnumerator* m_deviceEnumerator;

    bool setProcessVolume(DWORD processId, float volume);
    std::wstring getProcessNameById(DWORD processId);
    std::vector<DWORD> getRunningProcessIds();
};
