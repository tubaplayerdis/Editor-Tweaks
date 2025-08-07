#pragma once
#include "Hook.h"
#include <SDK.hpp>
#include <Utils.hpp>
#include <iostream>
#include <string>
#include "../TypeReconstruct.h"

#define F_GET_MAX_BRICK_SIZE (BASE + 0x0C3CA00)
#define F_UPDATE_PROPERTIES_PANEL (BASE + 0x0DA3040)
#define F_GET_VALUE_RANGE (BASE + 0x0BF3B80)
#define F_GET_SELECTED_PROPERTY_CONTAINER (BASE + 0x0DBFC70)
#define G_GLOBAL_GEAR_RATIO BASE + 0x42EB200
#define G_GLOBAL_LIGHT_BRIGHTNESS BASE + 0x42EB088 //Do the same stuff to light brightness as gear ratio

HOOK(GetMaxBrickSize, F_GET_MAX_BRICK_SIZE, [](SDK::UScalableBrick *This, SDK::FVector* RetVal) -> SDK::FVector*
{
    RetVal->X = FLT_MAX;
    RetVal->Y = FLT_MAX;
    RetVal->Z = FLT_MAX;
    return RetVal;
}, SDK::FVector*(SDK::UScalableBrick*, SDK::FVector*))

HOOK(UpdatePropertiesPanel, F_UPDATE_PROPERTIES_PANEL, [](SDK::UBrickEditorWidget* This) -> void
{
    auto GlobalGearRatioAddr = reinterpret_cast<FSharedRef*>(G_GLOBAL_GEAR_RATIO);
    if (GlobalGearRatioAddr->obj)
    {
        auto GlobalGearRatio = reinterpret_cast<FNumericBrickPropertyBase*>(GlobalGearRatioAddr->obj);
        GlobalGearRatio->Range.opt.Value.Max.Data.X = 20;
        GlobalGearRatio->Range.opt.Value.Max.Data.Y = 20;
        GlobalGearRatio->Range.opt.Value.Max.Data.Z = 20;
    }

    HOOK_CALL_ORIGINAL(H_UpdatePropertiesPanel, This);
}, void(SDK::UBrickEditorWidget*))