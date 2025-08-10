#pragma once
#include "Hook.h"
#include <SDK.hpp>
#include "../TypeReconstruct.h"

#define F_REFLECT_BRICK_PROPERTIES_MOTOR_BRICK (BASE + 0x0C58560)
#define F_REFLECT_BRICK_PROPERTIES_LIGHT_BRICK (BASE + 0x0C55A90)
#define F_REFLECT_BRICK_PROPERTIES_THRUSTER_BRICK (BASE + 0x0C61FD0)

#define G_GLOBAL_GEAR_RATIO (BASE + 0x42EB200)
#define G_GLOBAL_LIGHT_BRIGHTNESS (BASE + 0x42EB088)
#define G_GLOBAL_THRUSTER_INPUT_SCALE (BASE + 0x42EB868)
#define G_GLOBAL_THRUSTER_FORCE_PROPERTY (BASE + 0x42EB880)

constexpr float MAX_GEAR_REDUCTION = 1000.0f;
constexpr float MIN_GEAR_REDUCTION = 0.001f;
constexpr float MAX_BRIGHTNESS = 1000.f;
constexpr float MIN_BRIGHTNESS = 0.001f;
constexpr float MAX_THRUSTER_SCALE = 2000.0f;
constexpr float MIN_THRUSTER_SCALE = -2000.0f;

inline void SetNumericPropertyMinMax(FNumericBrickPropertyBase* Base, float Min, float Max)
{
    Base->Range.opt.Value.Max.Data.X = Max;
    Base->Range.opt.Value.Max.Data.Y = Max;
    Base->Range.opt.Value.Max.Data.Z = Max;

    Base->Range.opt.Value.Min.Data.X = Min;
    Base->Range.opt.Value.Min.Data.Y = Min;
    Base->Range.opt.Value.Min.Data.Z = Min;
}

HOOK(UMotorBrick_ReflectProperties, F_REFLECT_BRICK_PROPERTIES_MOTOR_BRICK, [](SDK::UMotorBrick* This, void *Params) -> void
{
    HOOK_CALL_ORIGINAL(UMotorBrick_ReflectProperties(), This, Params);
    auto GlobalGearRatioRef = reinterpret_cast<FSharedRef*>(G_GLOBAL_GEAR_RATIO);
    if (GlobalGearRatioRef->obj)
    {
        auto GlobalGearRatio = static_cast<FNumericBrickPropertyBase*>(GlobalGearRatioRef->obj);
        SetNumericPropertyMinMax(GlobalGearRatio, MIN_GEAR_REDUCTION, MAX_GEAR_REDUCTION);
    }
}, void(SDK::UMotorBrick*, void*))

HOOK(ULightBrick_ReflectProperties, F_REFLECT_BRICK_PROPERTIES_LIGHT_BRICK, [](SDK::ULightBrick* This, void *Params) -> void
{
    HOOK_CALL_ORIGINAL(ULightBrick_ReflectProperties(), This, Params);
    auto GlobalLightBrightnessRef = reinterpret_cast<FSharedRef*>(G_GLOBAL_LIGHT_BRIGHTNESS);
    if (GlobalLightBrightnessRef->obj)
    {
        auto GlobalLightBrightness = static_cast<FNumericBrickPropertyBase*>(GlobalLightBrightnessRef->obj);
        SetNumericPropertyMinMax(GlobalLightBrightness, MIN_GEAR_REDUCTION, MAX_GEAR_REDUCTION);
    }
}, void(SDK::ULightBrick*, void*))

HOOK(UThrusterBrick_RelfectProperties, F_REFLECT_BRICK_PROPERTIES_THRUSTER_BRICK, [](SDK::UThrusterBrick* This, void* Params) -> void
{
    HOOK_CALL_ORIGINAL(UThrusterBrick_RelfectProperties(), This, Params);
    auto GlobalThrusterScaleRef = reinterpret_cast<FSharedRef*>(G_GLOBAL_THRUSTER_INPUT_SCALE);
    if (GlobalThrusterScaleRef->obj)
    {
        auto GlobalThrusterScale = static_cast<FNumericBrickPropertyBase*>(GlobalThrusterScaleRef->obj);
        std::cout << GlobalThrusterScale->Range.opt.Value.Max.Data.X << "\n";//Reads wierd values
        std::cout << GlobalThrusterScale->Range.opt.Value.Max.Data.Y << "\n";
        std::cout << GlobalThrusterScale->Range.opt.Value.Max.Data.Z << "\n";
        std::cout << GlobalThrusterScale->Range.opt.Value.Min.Data.X << "\n";
        std::cout << GlobalThrusterScale->Range.opt.Value.Min.Data.Y << "\n";
        std::cout << GlobalThrusterScale->Range.opt.Value.Min.Data.Z << "\n";
        SetNumericPropertyMinMax(GlobalThrusterScale, MIN_THRUSTER_SCALE, MAX_THRUSTER_SCALE);
    }
}, void(SDK::UThrusterBrick*, void*));