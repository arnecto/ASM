#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys_devpkey.h>

struct AudioDevice {
    std::wstring id;
    std::wstring name;
    bool isDefault;
    bool isPlayback;
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool initialize();
    void cleanup();

    std::vector<AudioDevice> getPlaybackDevices();
    std::vector<AudioDevice> getCaptureDevices();

    bool setDefaultDevice(const std::wstring& deviceId, bool isPlayback);
    std::wstring getDefaultDeviceId(bool isPlayback);

    bool setDeviceVolume(const std::wstring& deviceId, float volume);
    float getDeviceVolume(const std::wstring& deviceId);

private:
    IMMDeviceEnumerator* m_deviceEnumerator;

    std::vector<AudioDevice> enumerateDevices(EDataFlow dataFlow);
    std::wstring getDeviceName(IMMDevice* device);
};
