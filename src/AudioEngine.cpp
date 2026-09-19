#include "AudioEngine.h"
#include <comdef.h>
#include <Mmdeviceapi.h>
#include <Policyconfigclient.h>
#include <Propidl.h>

AudioEngine::AudioEngine() : m_deviceEnumerator(nullptr) {}

AudioEngine::~AudioEngine() {
    cleanup();
}

bool AudioEngine::initialize() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return false;
    }

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&m_deviceEnumerator
    );

    return SUCCEEDED(hr);
}

void AudioEngine::cleanup() {
    if (m_deviceEnumerator) {
        m_deviceEnumerator->Release();
        m_deviceEnumerator = nullptr;
    }
    CoUninitialize();
}

std::vector<AudioDevice> AudioEngine::getPlaybackDevices() {
    return enumerateDevices(eRender);
}

std::vector<AudioDevice> AudioEngine::getCaptureDevices() {
    return enumerateDevices(eCapture);
}

std::vector<AudioDevice> AudioEngine::enumerateDevices(EDataFlow dataFlow) {
    std::vector<AudioDevice> devices;

    if (!m_deviceEnumerator) {
        return devices;
    }

    IMMDeviceCollection* deviceCollection = nullptr;
    HRESULT hr = m_deviceEnumerator->EnumAudioEndpoints(
        dataFlow,
        DEVICE_STATE_ACTIVE,
        &deviceCollection
    );

    if (FAILED(hr)) {
        return devices;
    }

    UINT count = 0;
    deviceCollection->GetCount(&count);

    IMMDevice* defaultDevice = nullptr;
    m_deviceEnumerator->GetDefaultAudioEndpoint(
        dataFlow,
        eConsole,
        &defaultDevice
    );

    LPWSTR defaultDeviceId = nullptr;
    if (defaultDevice) {
        defaultDevice->GetId(&defaultDeviceId);
    }

    for (UINT i = 0; i < count; i++) {
        IMMDevice* device = nullptr;
        hr = deviceCollection->Item(i, &device);

        if (SUCCEEDED(hr)) {
            LPWSTR deviceId = nullptr;
            device->GetId(&deviceId);

            AudioDevice audioDevice;
            audioDevice.id = deviceId;
            audioDevice.name = getDeviceName(device);
            audioDevice.isDefault = defaultDeviceId && (wcscmp(deviceId, defaultDeviceId) == 0);
            audioDevice.isPlayback = (dataFlow == eRender);

            devices.push_back(audioDevice);

            CoTaskMemFree(deviceId);
            device->Release();
        }
    }

    if (defaultDeviceId) {
        CoTaskMemFree(defaultDeviceId);
    }
    if (defaultDevice) {
        defaultDevice->Release();
    }

    deviceCollection->Release();
    return devices;
}

std::wstring AudioEngine::getDeviceName(IMMDevice* device) {
    IPropertyStore* propertyStore = nullptr;
    HRESULT hr = device->OpenPropertyStore(STGM_READ, &propertyStore);

    if (FAILED(hr)) {
        return L"Unknown Device";
    }

    PROPVARIANT varName;
    PropVariantInit(&varName);

    hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &varName);
    std::wstring name = L"Unknown Device";

    if (SUCCEEDED(hr) && varName.vt == VT_LPWSTR) {
        name = varName.pwszVal;
    }

    PropVariantClear(&varName);
    propertyStore->Release();

    return name;
}

bool AudioEngine::setDefaultDevice(const std::wstring& deviceId, bool isPlayback) {
    if (!m_deviceEnumerator) {
        return false;
    }

    IPolicyConfigVista* pPolicyConfig = nullptr;
    HRESULT hr = CoCreateInstance(
        __uuidof(CPolicyConfigVistaClient),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IPolicyConfigVista),
        (LPVOID*)&pPolicyConfig
    );

    if (FAILED(hr)) {
        return false;
    }

    ERole role = eConsole;
    hr = pPolicyConfig->SetDefaultEndpoint(deviceId.c_str(), role);

    pPolicyConfig->Release();
    return SUCCEEDED(hr);
}

std::wstring AudioEngine::getDefaultDeviceId(bool isPlayback) {
    if (!m_deviceEnumerator) {
        return L"";
    }

    IMMDevice* defaultDevice = nullptr;
    HRESULT hr = m_deviceEnumerator->GetDefaultAudioEndpoint(
        isPlayback ? eRender : eCapture,
        eConsole,
        &defaultDevice
    );

    if (FAILED(hr)) {
        return L"";
    }

    LPWSTR deviceId = nullptr;
    defaultDevice->GetId(&deviceId);

    std::wstring id = deviceId ? deviceId : L"";

    if (deviceId) {
        CoTaskMemFree(deviceId);
    }
    defaultDevice->Release();

    return id;
}

bool AudioEngine::setDeviceVolume(const std::wstring& deviceId, float volume) {
    if (!m_deviceEnumerator) {
        return false;
    }

    IMMDevice* device = nullptr;
    HRESULT hr = m_deviceEnumerator->GetDevice(deviceId.c_str(), &device);

    if (FAILED(hr)) {
        return false;
    }

    IAudioEndpointVolume* endpointVolume = nullptr;
    hr = device->Activate(
        __uuidof(IAudioEndpointVolume),
        CLSCTX_ALL,
        nullptr,
        (void**)&endpointVolume
    );

    if (SUCCEEDED(hr)) {
        volume = (std::max)(0.0f, (std::min)(1.0f, volume));
        endpointVolume->SetMasterVolumeLevelScalar(volume, nullptr);
        endpointVolume->Release();
    }

    device->Release();
    return SUCCEEDED(hr);
}

float AudioEngine::getDeviceVolume(const std::wstring& deviceId) {
    if (!m_deviceEnumerator) {
        return 0.0f;
    }

    IMMDevice* device = nullptr;
    HRESULT hr = m_deviceEnumerator->GetDevice(deviceId.c_str(), &device);

    if (FAILED(hr)) {
        return 0.0f;
    }

    IAudioEndpointVolume* endpointVolume = nullptr;
    hr = device->Activate(
        __uuidof(IAudioEndpointVolume),
        CLSCTX_ALL,
        nullptr,
        (void**)&endpointVolume
    );

    float volume = 0.0f;
    if (SUCCEEDED(hr)) {
        endpointVolume->GetMasterVolumeLevelScalar(&volume);
        endpointVolume->Release();
    }

    device->Release();
    return volume;
}
