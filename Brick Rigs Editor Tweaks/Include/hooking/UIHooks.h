#pragma once

#include "Hook.h"
#include <SDK.hpp>
#include <Utils.hpp>
#include <iostream>
#include <string>
#include "../TypeReconstruct.h"

#define F_INITIALIZE_CONTEXT_MENU (BASE + 0x0DC8F40)
#define F_ON_ACTION_PRESSED (BASE + 0x0DD7380)

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

HOOK(InitializeContextMenu, F_INITIALIZE_CONTEXT_MENU, [](SDK::UContextMenuWidget* This, FContextMenuParams* Context) -> void
{
    HOOK_CALL_ORIGINAL(InitializeContextMenu(), This, Context);
    if (!Context) return;
    if (Context->Text.ToString() == "Selection")
    {
        SDK::UWBP_EnumProperty_C* EnumProperty = CreateWidget(SDK::UWBP_EnumProperty_C);
        SDK::FBrickComboBoxItemParams Params = SDK::FBrickComboBoxItemParams();
        EnumProperty->ComboBox->InitItems(5, 1);
        EnumProperty->SetIsEnabled(true);
        EnumProperty->ComboBox->SetIsEnabled(true);
        for (SDK::UWidget* Widget : EnumProperty->ComboBox->ItemsPanel->GetAllChildren())
        {
            Widget->SetIsEnabled(true);
            static_cast<SDK::UWBP_BrickComboBoxItemContainer_C*>(Widget)->ItemWidget->TextBlock->SetText(TEXT(L"Sup!"));
        }
        This->ActionListWidget->MainGridPanel->AddChildToGrid(EnumProperty, 11, 0);

        for (SDK::UWidget* Widget : This->ActionListWidget->MainGridPanel->GetAllChildren())
        {
            if (CurrentActionRef != nullptr && Widget == CurrentActionRef) return;//Stop from adding advanced selection buttons
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
    }
}, void(SDK::UContextMenuWidget*, FContextMenuParams*));

HOOK(OnActionPresed, F_ON_ACTION_PRESSED, [](SDK::UInputActionWidget* This) -> void
{
    HOOK_CALL_ORIGINAL(OnActionPresed(), This);
    if (CurrentActionRef != nullptr && This == CurrentActionRef)
    {
        std::cout << "we should open a restaurant!\n";
    }
}, void(SDK::UInputActionWidget*))