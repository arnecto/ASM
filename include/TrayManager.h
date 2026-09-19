#pragma once

#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <functional>
#include <vector>

struct TrayMenuItem {
    std::wstring text;
    UINT id;
    bool separator;
    std::function<void()> callback;
};

class TrayManager {
public:
    TrayManager(HINSTANCE hInstance);
    ~TrayManager();

    bool initialize(const std::wstring& tooltip);
    void cleanup();

    void setMenuItems(const std::vector<TrayMenuItem>& items);
    void updateTooltip(const std::wstring& tooltip);

    void showNotification(const std::wstring& title, const std::wstring& message);

    HWND getWindow() const { return m_hwnd; }

private:
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    NOTIFYICONDATAW m_nid;
    std::vector<TrayMenuItem> m_menuItems;

    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void handleTrayMessage(LPARAM lParam);
    void showContextMenu();

    static constexpr UINT WM_TRAYICON = WM_USER + 1;
    static constexpr UINT TRAY_ICON_ID = 1;
};
