#include "TrayManager.h"
#include <shellapi.h>
#include <map>

static std::map<HWND, TrayManager*> g_trayInstances;

TrayManager::TrayManager(HINSTANCE hInstance)
    : m_hInstance(hInstance), m_hwnd(nullptr) {
    ZeroMemory(&m_nid, sizeof(m_nid));
}

TrayManager::~TrayManager() {
    cleanup();
}

bool TrayManager::initialize(const std::wstring& tooltip) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = windowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"AudioSwitchManagerTray";

    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        0,
        L"AudioSwitchManagerTray",
        L"AudioSwitchManager",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        m_hInstance,
        nullptr
    );

    if (!m_hwnd) {
        return false;
    }

    g_trayInstances[m_hwnd] = this;

    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = TRAY_ICON_ID;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcsncpy_s(m_nid.szTip, tooltip.c_str(), _TRUNCATE);

    return Shell_NotifyIconW(NIM_ADD, &m_nid) == TRUE;
}

void TrayManager::cleanup() {
    if (m_hwnd) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        g_trayInstances.erase(m_hwnd);
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

void TrayManager::setMenuItems(const std::vector<TrayMenuItem>& items) {
    m_menuItems = items;
}

void TrayManager::updateTooltip(const std::wstring& tooltip) {
    wcsncpy_s(m_nid.szTip, tooltip.c_str(), _TRUNCATE);
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayManager::showNotification(const std::wstring& title, const std::wstring& message) {
    m_nid.uFlags = NIF_INFO;
    wcsncpy_s(m_nid.szInfoTitle, title.c_str(), _TRUNCATE);
    wcsncpy_s(m_nid.szInfo, message.c_str(), _TRUNCATE);
    m_nid.dwInfoFlags = NIIF_INFO;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
}

LRESULT CALLBACK TrayManager::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto it = g_trayInstances.find(hwnd);
    if (it != g_trayInstances.end()) {
        TrayManager* manager = it->second;

        if (msg == WM_TRAYICON) {
            manager->handleTrayMessage(lParam);
            return 0;
        } else if (msg == WM_COMMAND) {
            UINT menuId = LOWORD(wParam);
            for (const auto& item : manager->m_menuItems) {
                if (item.id == menuId && item.callback) {
                    item.callback();
                    break;
                }
            }
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void TrayManager::handleTrayMessage(LPARAM lParam) {
    if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
        showContextMenu();
    }
}

void TrayManager::showContextMenu() {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();

    for (const auto& item : m_menuItems) {
        if (item.separator) {
            AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        } else {
            AppendMenuW(hMenu, MF_STRING, item.id, item.text.c_str());
        }
    }

    SetForegroundWindow(m_hwnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, m_hwnd, nullptr);
    DestroyMenu(hMenu);
}
