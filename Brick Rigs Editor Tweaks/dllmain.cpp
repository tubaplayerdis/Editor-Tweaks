/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Copyright (c) Aaron Wilk 2025, All rights reserved.                     */
/*                                                                            */
/*    Module:     dllmain.cpp                                                 */
/*    Author:     Aaron Wilk                                                  */
/*    Created:    24 June 2025                                                */
/*                                                                            */
/*    Revisions:  V0.1                                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include <windows.h>
#include <cstdio>
#include <MinHook.h>
#include "include/main/Main.h"

namespace
{
    FILE* pStdIn = nullptr;
    FILE* pStdOut = nullptr;
    FILE* pStdErr = nullptr;
}

DWORD WINAPI MainThread(const LPVOID lp_reserved)
{
    const auto h_module = static_cast<HMODULE>(lp_reserved);

#ifdef _DEBUG //If in debug version enable console.
    AllocConsole();
    (void)freopen_s(&pStdIn, "CONIN$", "r", stdin);
    (void)freopen_s(&pStdOut, "CONOUT$", "w", stdout);
    (void)freopen_s(&pStdErr, "CONOUT$", "w", stderr);
    SetConsoleTitleA(MOD_NAME);
    SetConsoleOutputCP(CP_UTF8);
#endif // _DEBUG

    MH_Initialize(); //Initalize MinHook

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST); //Prevent random freezes

    main_loop();

    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

#ifdef _DEBUG
    (void)fclose(pStdIn);
    (void)fclose(pStdOut);
    (void)fclose(pStdErr);
    SetStdHandle(STD_INPUT_HANDLE, nullptr);
    SetStdHandle(STD_OUTPUT_HANDLE, nullptr);
    SetStdHandle(STD_ERROR_HANDLE, nullptr);
    FreeConsole();
    PostMessage(GetConsoleWindow(), WM_CLOSE, 0, 0);
#endif

    FreeLibraryAndExitThread(h_module, 0);
    return 0;

}

BOOL APIENTRY DllMain(const HMODULE h_module, const DWORD  ul_reason_for_call, LPVOID lp_reserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(h_module);
        CreateThread(nullptr, 0, MainThread, h_module, 0, nullptr);
        return TRUE;
    }
    return FALSE;
}

