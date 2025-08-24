#pragma once

#include "Hook.h"
#include <SDK.hpp>
#include <Utils.hpp>
#include <iostream>
#include "../main/UIHelpers.h"
#include <string>
#include "../TypeReconstruct.h"

#define F_INITIALIZE_CONTEXT_MENU (BASE + 0x0DC8F40)
#define F_ON_ACTION_CLICKED (BASE + 0x0DD7300)
#define F_ON_ACTION_TRIGGERED (BASE + 0x0DD7420)

#undef TEXT
#define TEXT(text) SDK::UKismetTextLibrary::Conv_StringToText(SDK::FString(text))

struct FContextMenuParams
{
    SDK::FText Text;
    SDK::TWeakObjectPtr<SDK::UObject> Owner;
    SDK::TWeakObjectPtr<SDK::UInputCategory> InputCategory;
    SDK::TWeakObjectPtr<SDK::UClass> InputComponentClass;
};

inline SDK::UWBP_InputAction_C* CurrentActionRef = nullptr;
inline SDK::UContextMenuWidget* CurrentContextRef = nullptr;

HOOK(InitializeContextMenu, F_INITIALIZE_CONTEXT_MENU, [](SDK::UContextMenuWidget* This, FContextMenuParams* Context) -> void
{
    HOOK_CALL_ORIGINAL(InitializeContextMenu(), This, Context);
    if (!Context) return;
    if (Context->Text.ToString() == "Selection")
    {
        for (SDK::UWidget* Widget : This->ActionListWidget->MainGridPanel->GetAllChildren())
        {
            if (CurrentActionRef != nullptr && Widget == CurrentActionRef) return;//Stop from adding advanced selection button again.
        }

        SDK::UWBP_InputAction_C* InputAction = CreateWidget(SDK::UWBP_InputAction_C);
        CALL_GAME_FUNCTION(BASE + 0x0DC65E0, char(*)(SDK::UInputActionWidget*), InputAction);
        SDK::FDisplayInfo Info = SDK::FDisplayInfo();
        Info.Name = TEXT(L"Run Advanced Selection");
        InputAction->UpdateDisplayInfo(Info);
        InputAction->SetIsEnabled(true);
        InputAction->UpdateIsActionEnabled(true);
        InputAction->UpdateInputActionListMode(SDK::EInputActionListMode::ContextMenu);
        InputAction->Button->SetColorStyle(SDK::EBrickUIColorStyle::Highlight);
        This->ActionListWidget->AddActionWidget(InputAction, 12);//There are 11 button on the selection menu so 11 is the index at which one gets added.
        CurrentActionRef = InputAction;
        CurrentContextRef = This;
    }
}, void(SDK::UContextMenuWidget*, FContextMenuParams*));

HOOK(OnActionClicked, F_ON_ACTION_CLICKED, [](SDK::UInputActionWidget* This) -> void
{
    if (GetAsyncKeyState(VK_SHIFT) & 0x01 && IsActionNameValid(This))
    {
        if (This->Button->ColorStyle == SDK::EBrickUIColorStyle::Positive)
        {
            This->Button->SetColorStyle(SDK::EBrickUIColorStyle::Highlight);
            return;
        }
        This->Button->SetColorStyle(SDK::EBrickUIColorStyle::Positive);
        return;
    }
    HOOK_CALL_ORIGINAL(OnActionClicked(), This);

    if (CurrentActionRef != nullptr && This == CurrentActionRef && CurrentContextRef)
    {
        SelectValidObjects(CurrentContextRef->ActionListWidget);
    }

}, void(SDK::UInputActionWidget*))

HOOK(OnActionTriggered, F_ON_ACTION_TRIGGERED, [](SDK::UContextMenuWidget* This, const SDK::FName* ActionName, bool bReleased) -> void
{
    if (IsActionNameValid_N(ActionName) && bReleased) return;
    //The original does nothing
}, void(SDK::UContextMenuWidget*, const SDK::FName*, bool));