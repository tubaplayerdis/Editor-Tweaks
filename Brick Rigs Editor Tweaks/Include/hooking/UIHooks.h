#pragma once

#include "Hook.h"
#include <SDK.hpp>
#include <Utils.hpp>
#include <iostream>
#include <string>
#include "../TypeReconstruct.h"

#define F_INITIALIZE_CONTEXT_MENU (BASE + 0x0DC8F40)
#define F_ON_ACTION_CLICKED (BASE + 0x0DD7300)
#define F_ON_ACTION_TRIGGERED (BASE + 0x0DD7420)
#define F_GET_ACTIVE_OBJECT (BASE + 0x0BCC850)
#define F_SELECT_OBJECTS (BASE + 0x0C0AE50)

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
    }
}, void(SDK::UContextMenuWidget*, FContextMenuParams*));

inline bool IsActionNameValid(SDK::UInputActionWidget* Input)
{
    SDK::FName Name = GetMember<SDK::FName>(Input, 0x27C);
    if (Name.ToString() == "SelectByClass") return true;
    if (Name.ToString() == "SelectByColor") return true;
    if (Name.ToString() == "SelectByMaterial") return true;
    if (Name.ToString() == "SelectByPattern") return true;
    if (Name.ToString() == "SelectByType") return true;
    return false;
}

inline bool IsActionNameValid(const SDK::FName* Name)
{
    if (!Name) return false;
    if (Name->ToString() == "SelectByClass") return true;
    if (Name->ToString() == "SelectByColor") return true;
    if (Name->ToString() == "SelectByMaterial") return true;
    if (Name->ToString() == "SelectByPattern") return true;
    if (Name->ToString() == "SelectByType") return true;
    return false;
}

HOOK(OnActionClicked, F_ON_ACTION_CLICKED, [](SDK::UInputActionWidget* This) -> void
{
    if (GetAsyncKeyState(VK_SHIFT) & 0x01 && IsActionNameValid(This))
    {
        This->Button->SetColorStyle(SDK::EBrickUIColorStyle::Positive);
        return;
    }
    HOOK_CALL_ORIGINAL(OnActionClicked(), This);

    if (CurrentActionRef != nullptr && This == CurrentActionRef)
    {
        std::cout << "we should open a restaurant!\n";
    }

}, void(SDK::UInputActionWidget*))

inline bool IsColorSame(SDK::FColor Color1, SDK::FColor Color2)
{
    return Color1.A == Color2.A && Color1.R == Color2.R && Color1.G == Color2.G && Color1.B == Color2.B;
}

inline void SelectValidObjects(SDK::UInputActionListWidget* This)
{
    //Do we add these to the parameters
    bool sClass = false;
    bool sColor = false;
    bool sMaterial = false;
    bool sPattern = false;
    bool sType = false;

    //1. Get all the "highlighted" buttons and their action names.
    for (SDK::UWidget* _Widget : This->MainGridPanel->GetAllChildren())
    {
        SDK::UInputActionWidget* Widget = static_cast<SDK::UInputActionWidget*>(_Widget);

        if (Widget->Button->ColorStyle == SDK::EBrickUIColorStyle::Positive)
        {
            //Remove the highlighting.
            Widget->Button->SetColorStyle(SDK::EBrickUIColorStyle::Highlight);
            SDK::FName Name = GetMember<SDK::FName>(Widget, 0x27C);
            std::cout << Name.ToString() << std::endl;
            if (Name.ToString() == "SelectByClass") sClass = true;
            if (Name.ToString() == "SelectByColor") sColor = true;
            if (Name.ToString() == "SelectByMaterial") sMaterial = true;
            if (Name.ToString() == "SelectByPattern") sPattern = true;
            if (Name.ToString() == "SelectByType") sType = true;
        }
    }

    if (!sClass && !sColor && !sMaterial && !sPattern && !sType) return;

    //2. Get the editor
    SDK::ABrickEditor* ActiveEditor = nullptr;
    for (int i = 0; i < SDK::UObject::GObjects->Num(); i++)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);

        if (!Obj || Obj->IsDefaultObject())
            continue;

        if (Obj->IsA(SDK::ABrickEditor::StaticClass()))
        {
            ActiveEditor = static_cast<SDK::ABrickEditor*>(Obj);
        }
    }
    if (!ActiveEditor || ActiveEditor->EditorInterfaceComponent) return;

    //3. Get the active object
    auto ActiveObject = static_cast<SDK::UBrick*>(CallGameFunction<SDK::UBrickEditorObject*, SDK::ABrickEditor*>(F_GET_ACTIVE_OBJECT, ActiveEditor));
    if (!ActiveObject) return;

    std::cout << ActiveObject->GetName() << std::endl;

    //4. Iterate and find.
    std::vector<SDK::UBrickEditorObject*> Objects;
    for (SDK::UBrickEditorObject* _Object : ActiveEditor->EditorInterfaceComponent->BrickEditorObjects)
    {
        //Objects are searched for cases based and excluded upon first mis-matched case.

        if (!_Object->IsA(SDK::UBrick::StaticClass())) return;
        SDK::UBrick* Object = static_cast<SDK::UBrick*>(_Object);

        if (sClass)
        {
            if (ActiveObject->StaticClass() != Object->StaticClass()) continue; //The actual static class of the object.
        }

        if (sColor)
        {
            if (!IsColorSame(ActiveObject->BrickColor, Object->BrickColor)) continue;
        }

        if (sMaterial)
        {
            if (ActiveObject->BrickMaterial != Object->BrickMaterial) continue;
        }

        if (sPattern)
        {
            if (ActiveObject->BrickPattern != Object->BrickPattern) continue;
        }

        if (sType)
        {
            if (ActiveObject->GetStaticInfoClass() != Object->GetStaticInfoClass()) continue;//the static info classes are like UProperllerBrick
        }

        Objects.push_back(ActiveObject);
    }

    //5. Copy vector to array and pass to selection function.
    auto NewObjects = SDK::TAllocatedArray<SDK::UBrickEditorObject*>(Objects.size());
    for (int i = 0; i < Objects.size(); i++)
    {
        NewObjects[i] = Objects[i];
    }
    CallGameFunction<void, SDK::UBrickEditorMode*, const SDK::TArray<SDK::UBrickEditorObject*>*, bool>(F_SELECT_OBJECTS, ActiveEditor->CurrentEditorMode, &NewObjects, true);

}

HOOK(OnActionTriggered, F_ON_ACTION_TRIGGERED, [](SDK::UContextMenuWidget* This, const SDK::FName* ActionName, bool bReleased) -> void
{
    if (IsActionNameValid(ActionName) && bReleased) return;
    if (ActionName->ToString() == "Run Advanced Selection")
    {
        SelectValidObjects(This->ActionListWidget);
    }
    //The original does nothing
}, void(SDK::UContextMenuWidget*, const SDK::FName*, bool));