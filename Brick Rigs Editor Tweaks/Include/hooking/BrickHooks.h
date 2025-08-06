#pragma once
#include "Hook.h"
#include <SDK.hpp>
#include <Utils.hpp>
#include <iostream>

#define F_GET_MAX_BRICK_SIZE (BASE + 0x0C3CA00)
#define F_ON_SELECTION_CHANGED (BASE + 0x0D79960)
#define M_CURRENT_EDITOR_MODE (0x274)

HOOK(GetMaxBrickSize, F_GET_MAX_BRICK_SIZE, [](SDK::UScalableBrick *This, SDK::FVector* RetVal) -> SDK::FVector*
{
    RetVal->X = FLT_MAX;
    RetVal->Y = FLT_MAX;
    RetVal->Z = FLT_MAX;
    return RetVal;
}, SDK::FVector*(SDK::UScalableBrick*, SDK::FVector*))


HOOK(OnSelectionChanged, F_ON_SELECTION_CHANGED, [](SDK::UBrickEditorWidget* This) -> void
{
    std::cout << "Editor Mode Changed!\n";
    HOOK_CALL_ORIGINAL(H_OnSelectionChanged, This);
    std::cout << This->CurrentModeWidget->GetName() << "\n";
    SDK::TWeakObjectPtr<SDK::UBrickEditorMode> CurrentEditorMode = GetMember<SDK::TWeakObjectPtr<SDK::UBrickEditorMode>>(This, M_CURRENT_EDITOR_MODE);
    std::cout << CurrentEditorMode.Get()->GetName() << "\n";
}, void(SDK::UBrickEditorWidget*))