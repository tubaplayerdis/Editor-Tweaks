#pragma once

#include "Hook.h"
#include <SDK.hpp>
#include <Utils.hpp>
#include <iostream>
#include <string>
#include "../TypeReconstruct.h"

#define F_OPEN_INPUT_CATEGORY (BASE + 0x0DDC3A0)

HOOK(OpenInputCategory, F_OPEN_INPUT_CATEGORY, [](SDK::UWBP_InputActionList_C* This, SDK::UInputCategory* Category) -> void
{
    HOOK_CALL_ORIGINAL(OpenInputCategory(), This, Category);
    if (!Category) return;
    std::cout << "Open Category: " << Category->DisplayInfo.Name.ToString() << "\n";
}, void(SDK::UWBP_InputActionList_C*, SDK::UInputCategory*))