#pragma once

#include "Hook.h"
#include <SDK.hpp>
#include <Utils.hpp>
#include <iostream>
#include <string>
#include "../TypeReconstruct.h"

#define F_OPEN_CONTEXT_MENU (BASE + 0x0CA84B0)

#undef TEXT
#define TEXT(text) SDK::UKismetTextLibrary::Conv_StringToText(SDK::FString(text))

struct FContextMenuParams
{
    SDK::FText Text;
    SDK::TWeakObjectPtr<SDK::UObject> Owner;
    SDK::TWeakObjectPtr<SDK::UInputCategory> InputCategory;
    SDK::TWeakObjectPtr<SDK::UClass> InputComponentClass;
};

HOOK(OpenContextMenu, F_OPEN_CONTEXT_MENU, [](SDK::UWBP_WindowManager_C* This, FContextMenuParams* Context) -> bool
{
    bool ret = HOOK_CALL_ORIGINAL(OpenContextMenu(), This, Context);
    if (!Context) return ret;
    if (Context->Text.ToString() == "Selection")
    {
        std::cout << Context->Owner.Get()->GetName() << "\n";
    }
    return ret;
}, bool(SDK::UWBP_WindowManager_C*, FContextMenuParams*));