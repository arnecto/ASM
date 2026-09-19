#include "AudioEngine.h"
#include "TrayManager.h"
#include "HotkeyManager.h"
#include "ProcessVolumeManager.h"
#include "Config.h"
#include <Windows.h>
#include <memory>

enum MenuIds {
    MENU_EXIT = 1000,
    MENU_SETTINGS,
    MENU_PLAYBACK_DEVICES,
    MENU_CAPTURE_DEVICES,
    MENU_SEPARATOR,
    MENU_ABOUT,
    MENU_RELOAD_CONFIG,
    MENU_DEVICE_START = 2000,
    MENU_PROFILE_START = 3000
};

enum HotkeyIds {
    HOTKEY_NEXT_PLAYBACK = 1,
    HOTKEY_PREV_PLAYBACK,
    HOTKEY_NEXT_CAPTURE,
    HOTKEY_PREV_CAPTURE
};

struct AppContext {
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<TrayManager> trayManager;
    std::unique_ptr<HotkeyManager> hotkeyManager;
    std::unique_ptr<ProcessVolumeManager> processVolumeManager;
    std::unique_ptr<Config> config;
    bool running;
    std::vector<AudioDevice> playbackDevices;
    std::vector<AudioDevice> captureDevices;
};

static AppContext g_appContext;

void switchToNextDevice(bool isPlayback) {
    auto& devices = isPlayback ? g_appContext.playbackDevices : g_appContext.captureDevices;

    if (devices.empty()) {
        return;
    }

    std::wstring currentDeviceId = g_appContext.audioEngine->getDefaultDeviceId(isPlayback);

    size_t currentIndex = 0;
    for (size_t i = 0; i < devices.size(); i++) {
        if (devices[i].id == currentDeviceId) {
            currentIndex = i;
            break;
        }
    }

    size_t nextIndex = (currentIndex + 1) % devices.size();

    if (g_appContext.audioEngine->setDefaultDevice(devices[nextIndex].id, isPlayback)) {
        std::wstring message = L"Переключено на: " + devices[nextIndex].name;
        g_appContext.trayManager->showNotification(
            isPlayback ? L"Пристрій відтворення" : L"Пристрій запису",
            message
        );
    }
}

void switchToPreviousDevice(bool isPlayback) {
    auto& devices = isPlayback ? g_appContext.playbackDevices : g_appContext.captureDevices;

    if (devices.empty()) {
        return;
    }

    std::wstring currentDeviceId = g_appContext.audioEngine->getDefaultDeviceId(isPlayback);

    size_t currentIndex = 0;
    for (size_t i = 0; i < devices.size(); i++) {
        if (devices[i].id == currentDeviceId) {
            currentIndex = i;
            break;
        }
    }

    size_t prevIndex = currentIndex == 0 ? devices.size() - 1 : currentIndex - 1;

    if (g_appContext.audioEngine->setDefaultDevice(devices[prevIndex].id, isPlayback)) {
        std::wstring message = L"Переключено на: " + devices[prevIndex].name;
        g_appContext.trayManager->showNotification(
            isPlayback ? L"Пристрій відтворення" : L"Пристрій запису",
            message
        );
    }
}

void refreshDeviceLists() {
    g_appContext.playbackDevices = g_appContext.audioEngine->getPlaybackDevices();
    g_appContext.captureDevices = g_appContext.audioEngine->getCaptureDevices();
}

void loadConfiguration() {
    std::wstring configPath = g_appContext.config->getConfigFilePath();

    if (!g_appContext.config->load(configPath)) {
        HotkeyBindingConfig defaultHotkey1;
        defaultHotkey1.id = HOTKEY_NEXT_PLAYBACK;
        defaultHotkey1.modifiers = MOD_CONTROL | MOD_ALT;
        defaultHotkey1.vk = VK_NEXT;
        defaultHotkey1.action = L"next_playback";
        defaultHotkey1.description = L"Наступний пристрій відтворення";
        g_appContext.config->addHotkey(defaultHotkey1);

        HotkeyBindingConfig defaultHotkey2;
        defaultHotkey2.id = HOTKEY_PREV_PLAYBACK;
        defaultHotkey2.modifiers = MOD_CONTROL | MOD_ALT;
        defaultHotkey2.vk = VK_PRIOR;
        defaultHotkey2.action = L"prev_playback";
        defaultHotkey2.description = L"Попередній пристрій відтворення";
        g_appContext.config->addHotkey(defaultHotkey2);

        g_appContext.config->save(configPath);
    }

    for (const auto& hotkeyConfig : g_appContext.config->getHotkeys()) {
        if (hotkeyConfig.action == L"next_playback") {
            g_appContext.hotkeyManager->registerHotkey(
                hotkeyConfig.id,
                hotkeyConfig.modifiers,
                hotkeyConfig.vk,
                hotkeyConfig.description,
                []() { switchToNextDevice(true); }
            );
        } else if (hotkeyConfig.action == L"prev_playback") {
            g_appContext.hotkeyManager->registerHotkey(
                hotkeyConfig.id,
                hotkeyConfig.modifiers,
                hotkeyConfig.vk,
                hotkeyConfig.description,
                []() { switchToPreviousDevice(true); }
            );
        } else if (hotkeyConfig.action == L"next_capture") {
            g_appContext.hotkeyManager->registerHotkey(
                hotkeyConfig.id,
                hotkeyConfig.modifiers,
                hotkeyConfig.vk,
                hotkeyConfig.description,
                []() { switchToNextDevice(false); }
            );
        } else if (hotkeyConfig.action == L"prev_capture") {
            g_appContext.hotkeyManager->registerHotkey(
                hotkeyConfig.id,
                hotkeyConfig.modifiers,
                hotkeyConfig.vk,
                hotkeyConfig.description,
                []() { switchToPreviousDevice(false); }
            );
        }
    }

    for (const auto& volumeConfig : g_appContext.config->getProcessVolumes()) {
        g_appContext.processVolumeManager->addRule(volumeConfig.processName, volumeConfig.volume);
    }
}

void setupTrayMenu() {
    std::vector<TrayMenuItem> menuItems;

    TrayMenuItem playbackMenu;
    playbackMenu.text = L"📢 Пристрої відтворення";
    playbackMenu.id = MENU_PLAYBACK_DEVICES;
    playbackMenu.separator = false;
    menuItems.push_back(playbackMenu);

    refreshDeviceLists();
    UINT deviceMenuId = MENU_DEVICE_START;

    for (const auto& device : g_appContext.playbackDevices) {
        TrayMenuItem deviceItem;
        deviceItem.text = L"  " + device.name + (device.isDefault ? L" ✓" : L"");
        deviceItem.id = deviceMenuId++;
        deviceItem.separator = false;
        deviceItem.callback = [deviceId = device.id]() {
            g_appContext.audioEngine->setDefaultDevice(deviceId, true);
            g_appContext.trayManager->showNotification(L"Пристрій змінено", L"Пристрій відтворення успішно змінено");
            setupTrayMenu();
        };
        menuItems.push_back(deviceItem);
    }

    TrayMenuItem separator1;
    separator1.separator = true;
    menuItems.push_back(separator1);

    TrayMenuItem captureMenu;
    captureMenu.text = L"🎤 Пристрої запису";
    captureMenu.id = MENU_CAPTURE_DEVICES;
    captureMenu.separator = false;
    menuItems.push_back(captureMenu);

    for (const auto& device : g_appContext.captureDevices) {
        TrayMenuItem deviceItem;
        deviceItem.text = L"  " + device.name + (device.isDefault ? L" ✓" : L"");
        deviceItem.id = deviceMenuId++;
        deviceItem.separator = false;
        deviceItem.callback = [deviceId = device.id]() {
            g_appContext.audioEngine->setDefaultDevice(deviceId, false);
            g_appContext.trayManager->showNotification(L"Пристрій змінено", L"Пристрій запису успішно змінено");
            setupTrayMenu();
        };
        menuItems.push_back(deviceItem);
    }

    TrayMenuItem separator2;
    separator2.separator = true;
    menuItems.push_back(separator2);

    TrayMenuItem reloadConfig;
    reloadConfig.text = L"🔄 Перезавантажити конфігурацію";
    reloadConfig.id = MENU_RELOAD_CONFIG;
    reloadConfig.separator = false;
    reloadConfig.callback = []() {
        loadConfiguration();
        g_appContext.trayManager->showNotification(L"Конфігурація", L"Конфігурацію перезавантажено");
    };
    menuItems.push_back(reloadConfig);

    TrayMenuItem about;
    about.text = L"ℹ️ Про програму";
    about.id = MENU_ABOUT;
    about.separator = false;
    about.callback = []() {
        MessageBoxW(nullptr,
            L"AudioSwitchManager v1.0\n\nКерування аудіопристроями Windows\n\nГарячі клавіші:\nCtrl+Alt+PageDown - Наступний пристрій\nCtrl+Alt+PageUp - Попередній пристрій",
            L"Про програму",
            MB_OK | MB_ICONINFORMATION);
    };
    menuItems.push_back(about);

    TrayMenuItem separator3;
    separator3.separator = true;
    menuItems.push_back(separator3);

    TrayMenuItem exit;
    exit.text = L"❌ Вихід";
    exit.id = MENU_EXIT;
    exit.separator = false;
    exit.callback = []() {
        g_appContext.running = false;
        PostQuitMessage(0);
    };
    menuItems.push_back(exit);

    g_appContext.trayManager->setMenuItems(menuItems);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    g_appContext.audioEngine = std::make_unique<AudioEngine>();
    g_appContext.trayManager = std::make_unique<TrayManager>(hInstance);
    g_appContext.hotkeyManager = std::make_unique<HotkeyManager>();
    g_appContext.processVolumeManager = std::make_unique<ProcessVolumeManager>();
    g_appContext.config = std::make_unique<Config>();
    g_appContext.running = true;

    if (!g_appContext.audioEngine->initialize()) {
        MessageBoxW(nullptr, L"Не вдалося ініціалізувати аудіо підсистему", L"Помилка", MB_OK | MB_ICONERROR);
        return 1;
    }

    if (!g_appContext.processVolumeManager->initialize()) {
        MessageBoxW(nullptr, L"Не вдалося ініціалізувати менеджер гучності процесів", L"Помилка", MB_OK | MB_ICONERROR);
        return 1;
    }

    if (!g_appContext.trayManager->initialize(L"AudioSwitchManager")) {
        MessageBoxW(nullptr, L"Не вдалося створити трей іконку", L"Помилка", MB_OK | MB_ICONERROR);
        return 1;
    }

    loadConfiguration();
    setupTrayMenu();

    g_appContext.trayManager->showNotification(
        L"AudioSwitchManager",
        L"Програму запущено. Використовуйте Ctrl+Alt+PageUp/PageDown для перемикання пристроїв."
    );

    SetTimer(g_appContext.trayManager->getWindow(), 1, 2000, nullptr);

    MSG msg = {};
    while (g_appContext.running && GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            g_appContext.hotkeyManager->processHotkey(static_cast<int>(msg.wParam));
        } else if (msg.message == WM_TIMER && msg.wParam == 1) {
            g_appContext.processVolumeManager->applyRules();
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KillTimer(g_appContext.trayManager->getWindow(), 1);

    g_appContext.hotkeyManager->unregisterAll();
    g_appContext.processVolumeManager->cleanup();
    g_appContext.audioEngine->cleanup();
    g_appContext.trayManager->cleanup();

    return 0;
}
