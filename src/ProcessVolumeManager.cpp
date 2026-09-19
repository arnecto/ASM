#include "ProcessVolumeManager.h"
#include <psapi.h>
#include <endpointvolume.h>
#include <algorithm>
#include <tlhelp32.h>

#undef min
#undef max

ProcessVolumeManager::ProcessVolumeManager()
    : m_enabled(true), m_deviceEnumerator(nullptr) {}

ProcessVolumeManager::~ProcessVolumeManager() {
    cleanup();
}

bool ProcessVolumeManager::initialize() {
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&m_deviceEnumerator
    );

    return SUCCEEDED(hr);
}

void ProcessVolumeManager::cleanup() {
    if (m_deviceEnumerator) {
        m_deviceEnumerator->Release();
        m_deviceEnumerator = nullptr;
    }
}

void ProcessVolumeManager::addRule(const std::wstring& processName, float volume) {
    ProcessVolumeRule rule;
    rule.processName = processName;
    rule.targetVolume = std::max(0.0f, std::min(1.0f, volume));
    rule.enabled = true;

    m_rules[processName] = rule;
}

void ProcessVolumeManager::removeRule(const std::wstring& processName) {
    m_rules.erase(processName);
}

void ProcessVolumeManager::updateRule(const std::wstring& processName, float volume) {
    auto it = m_rules.find(processName);
    if (it != m_rules.end()) {
        it->second.targetVolume = std::max(0.0f, std::min(1.0f, volume));
    }
}

std::vector<ProcessVolumeRule> ProcessVolumeManager::getRules() const {
    std::vector<ProcessVolumeRule> rules;
    for (const auto& pair : m_rules) {
        rules.push_back(pair.second);
    }
    return rules;
}

void ProcessVolumeManager::applyRules() {
    if (!m_enabled || !m_deviceEnumerator) {
        return;
    }

    IMMDevice* device = nullptr;
    HRESULT hr = m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    if (FAILED(hr)) {
        return;
    }

    IAudioSessionManager2* sessionManager = nullptr;
    hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&sessionManager);

    if (SUCCEEDED(hr)) {
        IAudioSessionEnumerator* sessionEnumerator = nullptr;
        hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);

        if (SUCCEEDED(hr)) {
            int sessionCount = 0;
            sessionEnumerator->GetCount(&sessionCount);

            for (int i = 0; i < sessionCount; i++) {
                IAudioSessionControl* sessionControl = nullptr;
                hr = sessionEnumerator->GetSession(i, &sessionControl);

                if (SUCCEEDED(hr)) {
                    IAudioSessionControl2* sessionControl2 = nullptr;
                    hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);

                    if (SUCCEEDED(hr)) {
                        DWORD processId = 0;
                        sessionControl2->GetProcessId(&processId);

                        if (processId != 0) {
                            std::wstring processName = getProcessNameById(processId);

                            auto it = m_rules.find(processName);
                            if (it != m_rules.end() && it->second.enabled) {
                                ISimpleAudioVolume* audioVolume = nullptr;
                                hr = sessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&audioVolume);

                                if (SUCCEEDED(hr)) {
                                    audioVolume->SetMasterVolume(it->second.targetVolume, nullptr);
                                    audioVolume->Release();
                                }
                            }
                        }

                        sessionControl2->Release();
                    }

                    sessionControl->Release();
                }
            }

            sessionEnumerator->Release();
        }

        sessionManager->Release();
    }

    device->Release();
}

std::wstring ProcessVolumeManager::getProcessNameById(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (!hProcess) {
        return L"";
    }

    wchar_t processPath[MAX_PATH] = {};
    DWORD size = MAX_PATH;

    if (QueryFullProcessImageNameW(hProcess, 0, processPath, &size)) {
        std::wstring fullPath = processPath;
        size_t lastSlash = fullPath.find_last_of(L"\\");
        if (lastSlash != std::wstring::npos) {
            CloseHandle(hProcess);
            return fullPath.substr(lastSlash + 1);
        }
    }

    CloseHandle(hProcess);
    return L"";
}

std::vector<DWORD> ProcessVolumeManager::getRunningProcessIds() {
    std::vector<DWORD> processIds;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return processIds;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snapshot, &pe32)) {
        do {
            processIds.push_back(pe32.th32ProcessID);
        } while (Process32NextW(snapshot, &pe32));
    }

    CloseHandle(snapshot);
    return processIds;
}

bool ProcessVolumeManager::setProcessVolume(DWORD processId, float volume) {
    if (!m_deviceEnumerator) {
        return false;
    }

    IMMDevice* device = nullptr;
    HRESULT hr = m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    if (FAILED(hr)) {
        return false;
    }

    bool success = false;
    IAudioSessionManager2* sessionManager = nullptr;
    hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&sessionManager);

    if (SUCCEEDED(hr)) {
        IAudioSessionEnumerator* sessionEnumerator = nullptr;
        hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);

        if (SUCCEEDED(hr)) {
            int sessionCount = 0;
            sessionEnumerator->GetCount(&sessionCount);

            for (int i = 0; i < sessionCount; i++) {
                IAudioSessionControl* sessionControl = nullptr;
                hr = sessionEnumerator->GetSession(i, &sessionControl);

                if (SUCCEEDED(hr)) {
                    IAudioSessionControl2* sessionControl2 = nullptr;
                    hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);

                    if (SUCCEEDED(hr)) {
                        DWORD sessionProcessId = 0;
                        sessionControl2->GetProcessId(&sessionProcessId);

                        if (sessionProcessId == processId) {
                            ISimpleAudioVolume* audioVolume = nullptr;
                            hr = sessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&audioVolume);

                            if (SUCCEEDED(hr)) {
                                audioVolume->SetMasterVolume(volume, nullptr);
                                audioVolume->Release();
                                success = true;
                            }
                        }

                        sessionControl2->Release();
                    }

                    sessionControl->Release();
                }
            }

            sessionEnumerator->Release();
        }

        sessionManager->Release();
    }

    device->Release();
    return success;
}
