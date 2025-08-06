#pragma once
#include "Hook.h"
#include <SDK.hpp>
#include <Utils.hpp>
#include <iostream>
#include <string>

#define F_GET_MAX_BRICK_SIZE (BASE + 0x0C3CA00)
#define F_INITIALIZE_PROPERTY (BASE + 0x0DCB870)
#define F_GET_VALUE_RANGE (BASE + 0x0BF3B80)
#define F_GET_SELECTED_PROPERTY_CONTAINER (BASE + 0x0DBFC70)
#define M_CURRENT_EDITOR_MODE (0x26C)

struct FSharedRef
{
    void* obj;
    void* refenceController;
};

struct FNumericBrickPropertyValue
{
    SDK::FVector Data;
    unsigned __int8 NumUsed;
};
static_assert(sizeof(FNumericBrickPropertyValue) == 0x10);

struct FNumericBrickPropertyRange
{
    FNumericBrickPropertyValue Min;
    FNumericBrickPropertyValue Max;
};
static_assert(sizeof(FNumericBrickPropertyRange) == 0x20);


struct FOptional
{
    FNumericBrickPropertyRange Value;
    bool bIsSet;
    uint8_t pad[3];
};
static_assert(sizeof(FOptional) == 0x24);

struct FBrickPropAttribute //const TBrickPropAttribute<FNumericBrickPropertyRange> ValueRange;
{
    FOptional opt;
    uint8_t padn[4];
    uint8_t pad[0x10];
};
static_assert(sizeof(FBrickPropAttribute) == 0x38);

HOOK(GetMaxBrickSize, F_GET_MAX_BRICK_SIZE, [](SDK::UScalableBrick *This, SDK::FVector* RetVal) -> SDK::FVector*
{
    RetVal->X = FLT_MAX;
    RetVal->Y = FLT_MAX;
    RetVal->Z = FLT_MAX;
    return RetVal;
}, SDK::FVector*(SDK::UScalableBrick*, SDK::FVector*))

HOOK(InitializeProperty, F_INITIALIZE_PROPERTY, [](SDK::UPropertyContainerWidget* This, FSharedRef* InProps, SDK::EOrientation Orientation) -> void
{
    //InProps->obj is a FBrickPropertyEditInfo
    auto PropRef = GetMember<FSharedRef>(InProps->obj, 0x0);
    auto DisplayName = GetMember<SDK::FText>(InProps->obj, 0x40);
    if (DisplayName.ToString() == "Brightness")
    {
        void* BrickPropertyNumeric = PropRef.obj;//This is a FNumericBrickProperty which inherits FNumericBrickPropertyBase
        void* PropertyContainer = CALL_GAME_FUNCTION(F_GET_SELECTED_PROPERTY_CONTAINER, void*(*)(void*, void*))
        //auto BrickProperty = GetMember<FBrickPropAttribute>(BrickPropertyNumeric, 0x30);
        //BrickProperty.opt.Value.Max.Data.X = 2.0f;
        //BrickProperty.opt.Value.Max.Data.Y = 2.0f;
        //BrickProperty.opt.Value.Max.Data.Z = 2.0f;
        //BrickProperty.opt.Value.Min.Data.X;

    }
    void* BrickProperty = PropRef.obj;
    SDK::FName name;
    (CallVTableFunction<SDK::FName*, SDK::FName*>(0, BrickProperty, &name));
    std::cout << DisplayName.ToString() << ": " << name.ToString() << "\n";
    HOOK_CALL_ORIGINAL(H_InitializeProperty, This, InProps, Orientation);
}, void(SDK::UPropertyContainerWidget*, FSharedRef*, SDK::EOrientation))