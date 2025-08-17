#include "../Include/Cleanup.h"
#include "../Include/TypeReconstruct.h"
#include "../Include/hooking/DataHooks.h"

void SetNumericBrickPropertyMinMax(FNumericBrickPropertyBase* Base, float Min, float Max) //Same as set numeric property min max
{
    Base->Range.opt.Value.Max.Data.X = Max;
    Base->Range.opt.Value.Max.Data.Y = Max;
    Base->Range.opt.Value.Max.Data.Z = Max;

    Base->Range.opt.Value.Min.Data.X = Min;
    Base->Range.opt.Value.Min.Data.Y = Min;
    Base->Range.opt.Value.Min.Data.Z = Min;
}

void CleanupProperty(const FSharedRef* SharedReference, float min, float max)
{
    if (!SharedReference->obj) return;
    auto GlobalVar =  static_cast<FNumericBrickPropertyBase*>(SharedReference->obj);
    SetNumericBrickPropertyMinMax(GlobalVar, min, max);
}

#define GEAR_RATIO_MIN_NORM 0.1f
#define GEAR_RATIO_MAX_NORM 10.0f
#define LIGHT_BRIGHTNESS_MIN_NORM 0
#define LIGHT_BRIGHTNESS_MAX_NORM 0.1


void CleanupTweaks()
{
    CleanupProperty(reinterpret_cast<FSharedRef*>(G_GLOBAL_GEAR_RATIO), GEAR_RATIO_MIN_NORM, GEAR_RATIO_MAX_NORM);
    CleanupProperty(reinterpret_cast<FSharedRef*>(G_GLOBAL_LIGHT_BRIGHTNESS), GEAR_RATIO_MIN_NORM, GEAR_RATIO_MAX_NORM);
}