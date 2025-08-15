#include "../../Include/main/Main.h"
#include <windows.h>
#include "../../Include/hooking/BrickHooks.h"
#include "../../Include/hooking/DataHooks.h"

namespace
{
    bool is_active_window()
    {
        const HWND hwnd = GetForegroundWindow();
        if (hwnd == nullptr) return false;

        DWORD foreground_pid;
        if (GetWindowThreadProcessId(hwnd, &foreground_pid) == 0) return false;

        return (foreground_pid == GetCurrentProcessId());
    }
}

#define PRESSED 0x8000
#ifdef _DEBUG
#define UNINJECT_PRESS() GetAsyncKeyState(VK_DIVIDE) & 1
#else
#define UNINJECT_PRESS() (IsActiveWindow() && (GetAsyncKeyState(VK_CONTROL) & PRESSED) && (GetAsyncKeyState('U') & PRESSED))
#endif

#define CONTINUE_PRESS() (is_active_window() && (GetAsyncKeyState(VK_RETURN) & 1))

void main_loop()
{
    //Run initialization

    //Initialize Hooks
    HOOK_INIT(GetMaxBrickSize());
    HOOK_INIT(UpdatePropertiesPanel());
    HOOK_INIT(UMotorBrick_ReflectProperties());
    HOOK_INIT(ULightBrick_ReflectProperties());
    HOOK_INIT(GetThrustForceRange());

    //Enable Hooks
    HOOK_ENABLE(GetMaxBrickSize());
    HOOK_ENABLE(UpdatePropertiesPanel());
    HOOK_ENABLE(UMotorBrick_ReflectProperties());
    HOOK_ENABLE(ULightBrick_ReflectProperties());
    HOOK_ENABLE(GetThrustForceRange());

    while (true) {

        Sleep(10);

        if (CONTINUE_PRESS()) continue;

        if (UNINJECT_PRESS()) break;
    }

    //Destroy Hooks
    HOOK_DESTROY(GetMaxBrickSize());
    HOOK_DESTROY(UpdatePropertiesPanel());
    HOOK_DESTROY(UMotorBrick_ReflectProperties());
    HOOK_DESTROY(ULightBrick_ReflectProperties());
    HOOK_DESTROY(GetThrustForceRange());
    //Run Cleanup
}