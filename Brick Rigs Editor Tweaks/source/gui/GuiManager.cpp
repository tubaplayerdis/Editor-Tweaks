// --- Prevent Windows macros from polluting everything ---
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

// External libs
#include "../../Include/kiero/Kiero.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>

#include "../../Include/gui/GuiManager.h"
#include <vector>
#include <memory>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <atomic>

// ------------------------------------------------------------
// Typedefs
// ------------------------------------------------------------
typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------
static bool _is_init = false;
static Present oPresent = nullptr;
static HWND window = NULL;
static WNDPROC oWndProc = nullptr;
static ID3D11Device* pDevice = nullptr;
static ID3D11DeviceContext* pContext = nullptr;
static ID3D11RenderTargetView* mainRenderTargetView = nullptr;

static std::vector<gui_menu*> menus;
static std::shared_mutex menus_mutex;
static std::vector<gui_menu*> prev_menus = std::vector<gui_menu*>();
static std::unique_ptr<gui_manager> manager = nullptr;

// ------------------------------------------------------------
// WndProc Hook
// ------------------------------------------------------------
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

// ------------------------------------------------------------
// Hooked Present
// ------------------------------------------------------------
HRESULT __stdcall hooked_present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!_is_init)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
        {
            pDevice->GetImmediateContext(&pContext);

            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer = nullptr;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            if (pBackBuffer) pBackBuffer->Release();

            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

            // --- ImGui Init ---
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

            ImGui_ImplWin32_Init(window);
            ImGui_ImplDX11_Init(pDevice, pContext);

            _is_init = true;
        }
        else
        {
            return oPresent(pSwapChain, SyncInterval, Flags);
        }
    }

    // --- Start ImGui frame ---
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Render all registered menus
    std::shared_lock<std::shared_mutex> lock(menus_mutex);
    for (gui_menu* menu : menus)
    {
        if (menu && menu->is_visible)
            menu->menu();
    }

    // --- Render ---
    ImGui::Render();
    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

// ------------------------------------------------------------
// gui_manager implementation
// ------------------------------------------------------------
gui_manager::gui_manager()
{
    if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
    {
        kiero::bind(8, (void**)&oPresent, hooked_present); // 8 = Present
    }
}

gui_manager* gui_manager::get()
{
    if (!manager)
        manager = std::unique_ptr<gui_manager>(new gui_manager());
    return manager.get();
}

void gui_manager::shutdown()
{
    kiero::unbind(8);
    ImGui_ImplWin32_Shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui::DestroyContext();
    kiero::shutdown(8);

    if (oWndProc && window)
    {
        SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        oWndProc = nullptr;
    }

    if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = nullptr; }
    if (pContext) { pContext->Release(); pContext = nullptr; }
    if (pDevice) { pDevice->Release(); pDevice = nullptr; }

    manager.reset();

    _is_init = false;
}

void gui_manager::remove_menu(gui_menu* menu)
{
    for (size_t i = 0; i < menus.size(); i++)
    {
        if (menus[i] == menu)
        {
            menus.erase(menus.begin() + i);
            break;
        }
    }
}

void gui_manager::set_menu_visibility(gui_menu* menu, bool visibility)
{
    if (visibility)
    {
        add_menu(menu);
        if (menu->custom_toggle) menu->custom_toggle(visibility);
        menu->is_visible = visibility;
    }
    else
    {
        if (menu->custom_toggle) menu->custom_toggle(visibility);
        menu->is_visible = visibility;
    }

}

void gui_manager::toggle_menu_visibility(gui_menu* menu)
{
    set_menu_visibility(menu, !menu->is_visible);
}

void gui_manager::hide_all()
{
    for (gui_menu* menu : menus)
    {
        if (menu->is_visible) prev_menus.push_back(menu);
        menu->is_visible = false;
    }
}

bool gui_manager::are_all_hidden()
{
    return prev_menus.size() > 0;
}

void gui_manager::display_all_previous()
{
    for (gui_menu* menu : prev_menus)
    {
        menu->is_visible = true;
    }
    prev_menus.clear();
}

void gui_manager::add_menu(gui_menu* menu)
{
    for (gui_menu* menu_itor: menus)
    {
        if (menu_itor == menu) return;
    }
    menus.push_back(menu);
}