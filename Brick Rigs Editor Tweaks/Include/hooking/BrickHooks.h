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
#define F_GET_THRUST_FORCE_RANGE (BASE + 0x0C41990)
#define F_GET_STATIC_INFO_THRUSTER_BRICK (BASE + 0x0C1EE00)
#define F_GET_BRICK_EDITOR_STATIC_INFO (BASE + 0x0BF2710)

HOOK(GetMaxBrickSize, F_GET_MAX_BRICK_SIZE, [](SDK::UScalableBrick *This, SDK::FVector* RetVal) -> SDK::FVector*
{
    RetVal->X = FLT_MAX;
    RetVal->Y = FLT_MAX;
    RetVal->Z = FLT_MAX;
    return RetVal;
}, SDK::FVector*(SDK::UScalableBrick*, SDK::FVector*))

HOOK(UpdatePropertiesPanel, F_UPDATE_PROPERTIES_PANEL, [](SDK::UBrickEditorWidget* This) -> void
{
    //Call whatever
    HOOK_CALL_ORIGINAL(UpdatePropertiesPanel(), This);
}, void(SDK::UBrickEditorWidget*))

HOOK(GetThrustForceRange, F_GET_THRUST_FORCE_RANGE, [](SDK::UThrusterBrick* This, void* Params) -> void*
{
    SDK::UThrusterBrickStaticInfo* StaticInfo = CALL_GAME_FUNCTION(F_GET_STATIC_INFO_THRUSTER_BRICK, SDK::UThrusterBrickStaticInfo*(*)(SDK::UThrusterBrick*), This);//IDA incorrectly assumes that the first paramater is the class context object. I suspect this behavior will replicate on other static info getters.
    StaticInfo->MaxInputScale = 1000.0f;//Normally 25
    return HOOK_CALL_ORIGINAL(GetThrustForceRange(), This, Params);
}, void*(SDK::UThrusterBrick*, void*))

HOOK(GetBrickEditorStaticInfo, F_GET_BRICK_EDITOR_STATIC_INFO, [](SDK::ABrickEditor* This) -> SDK::UBrickEditorStaticInfo*
{
    SDK::UBrickEditorStaticInfo* StaticInfo = HOOK_CALL_ORIGINAL(GetBrickEditorStaticInfo(), This);
    StaticInfo->EditorBounds.X = 1000000; //100000 normally. Expanding to floating point limit causes Brick to be unable to be selected.
    StaticInfo->EditorBounds.Y = 1000000;
    StaticInfo->EditorBounds.Z = 1000000;
    StaticInfo->CameraSpeedParams.SpeedRange.Min = 10.0f; //300
    StaticInfo->CameraSpeedParams.SpeedRange.Max = 24000.0f; //24000
    StaticInfo->CameraSpeedParams.Step = 0.01f;//5
    return StaticInfo;
}, SDK::UBrickEditorStaticInfo*(SDK::ABrickEditor*))