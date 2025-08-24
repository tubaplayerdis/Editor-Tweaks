#include "../Brick Rigs Editor Tweaks/Include/main/UIHelpers.h"

#include <Helpers/GameFunctions.hpp>

#define F_GET_ACTIVE_OBJECT (BASE + 0x0BCC850)
#define F_SELECT_OBJECTS (BASE + 0x0C0AE50)
#define M_PARENT_WIDGET (0x270)
#define M_ACTION_NAME (0x27C)

bool IsActionNameValid(SDK::UInputActionWidget* Input)
{
    SDK::FName Name = GetMember<SDK::FName>(Input, 0x27C);
    if (Name.ToString() == "SelectByClass") return true;
    if (Name.ToString() == "SelectByColor") return true;
    if (Name.ToString() == "SelectByMaterial") return true;
    if (Name.ToString() == "SelectByPattern") return true;
    if (Name.ToString() == "SelectByType") return true;
    return false;
}


bool IsActionNameValid_N(const SDK::FName* Name)
{
    if (!Name) return false;
    if (Name->ToString() == "SelectByClass") return true;
    if (Name->ToString() == "SelectByColor") return true;
    if (Name->ToString() == "SelectByMaterial") return true;
    if (Name->ToString() == "SelectByPattern") return true;
    if (Name->ToString() == "SelectByType") return true;
    return false;
}

bool IsColorSame(SDK::FColor Color1, SDK::FColor Color2)
{
    return Color1.A == Color2.A && Color1.R == Color2.R && Color1.G == Color2.G && Color1.B == Color2.B;
}

void SelectValidObjects(SDK::UInputActionListWidget* This)
{
    std::cout << This << '\n';

    if (!This) return;

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
            SDK::FName Name = GetMember<SDK::FName>(Widget, M_ACTION_NAME);
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
    if (!ActiveObject)
    {
        std::cout << ActiveObject << '\n';
        return;
    }

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

SDK::UInputActionListWidget* GetParentWidget(SDK::UInputActionWidget* Widget)
{
    return GetMember<SDK::TWeakObjectPtr<SDK::UInputActionListWidget>>(Widget, 0x270).Get();
}
