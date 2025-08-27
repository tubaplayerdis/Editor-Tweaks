#include "../Brick Rigs Editor Tweaks/Include/main/UIHelpers.h"

#include <Utils/GameFunctions.hpp>

enum class EBrickSelectionState : uint8_t
{
    Unselected,
    Selected,
    Active
};



#define F_SELECT_OBJECTS (BASE + 0x0C0AB10)
#define M_PARENT_WIDGET (0x270)
#define M_ACTION_NAME (0x27C)
#define M_SELECTED_OBJECTS (0x3C8)

enum class ESelectObjectsMode : uint8_t
{
    AddToSelection = 0,
    MoveToStart = 1,
    RemoveFromSelection = 2,
    Destroyed = 3,
    ReplaceSelection = 4,
    ToggleSelection = 5
};

namespace FMemory
{
    constexpr std::size_t F_MALLOC_EXTERNAL = 0x0F25460;
    constexpr std::size_t F_FREE = 0x0F19030;

    void* Malloc(std::size_t size, unsigned int alignment)
    {
        return CallGameFunction<void*, size_t, unsigned int>(BASE + F_MALLOC_EXTERNAL, size, alignment);
    }

    void Free(void* ptr)
    {
        CallGameFunction<void, void*>(BASE + F_FREE, ptr);
    }
}

namespace FArray
{
    template <typename T>
    class TCustomArray
    {
        T* _Data;
        int _Max;
        int _Num;

        public:
        TCustomArray(int max)
        {
            _Data = reinterpret_cast<T*>(FMemory::Malloc(sizeof(T) * max, 0));
            _Max = max;
            _Num = 0;
        }

        void Add(T Member)
        {
            _Data[_Num] = Member;
            _Num++;
        }

        int Num()
        {
            return _Num;
        }
    };
}

static void SelectObjects(SDK::ABrickEditor* This, void* ObjectsToSelect, ESelectObjectsMode Mode)
{
    return CallGameFunction<void, SDK::ABrickEditor*, void*, ESelectObjectsMode, bool, bool>(F_SELECT_OBJECTS, This, ObjectsToSelect, Mode, false, true);
}

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
    if (!ActiveEditor || !ActiveEditor->EditorInterfaceComponent) return;

    //3. Get the active object
    SDK::TArray<SDK::TWeakObjectPtr<SDK::UBrickEditorObject>> SelectedObjects = GetMember<SDK::TArray<SDK::TWeakObjectPtr<SDK::UBrickEditorObject>>>(ActiveEditor, M_SELECTED_OBJECTS);
    if (SelectedObjects.Num() < 1) return;
    auto ActiveObject = Cast<SDK::UBrick>(SelectedObjects[0].Get());
    if (!ActiveObject)
    {
        std::cout << ActiveObject << '\n';
        return;
    }

    //4. Iterate and find.
    std::vector<SDK::UBrickEditorObject*> Objects;
    for (SDK::UBrickEditorObject* _Object : ActiveEditor->EditorInterfaceComponent->BrickEditorObjects)
    {
        //Objects are searched for cases based and excluded upon first mis-matched case.

        if (!_Object->IsA(SDK::UBrick::StaticClass()))
        {
            std::cout << "wrong type!" << '\n';
            return;
        }
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

        if (_Object == ActiveObject)
        {
            continue;
        }

        Objects.push_back(ActiveObject);
    }

    //Prevents a crash.
    auto NewObjects = FArray::TCustomArray<SDK::UBrickEditorObject*>(Objects.size());
    for (int i = 0; i < Objects.size(); i++)
    {
        NewObjects.Add(Objects[i]);
    }

    std::cout << NewObjects.Num() << '\n';

    //Still does not work. The custom TArray implementation works though
    //TODO: Fix and read note
    SelectObjects(ActiveEditor, &NewObjects, ESelectObjectsMode::AddToSelection);

    std::cout << ActiveEditor->CurrentEditorMode->GetName() << std::endl;
}

SDK::UInputActionListWidget* GetParentWidget(SDK::UInputActionWidget* Widget)
{
    return GetMember<SDK::TWeakObjectPtr<SDK::UInputActionListWidget>>(Widget, 0x270).Get();
}
