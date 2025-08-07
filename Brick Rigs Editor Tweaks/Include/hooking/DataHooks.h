#pragma once
#include "Hook.h"
#include <SDK.hpp>
#include <Utils.hpp>
#include <iostream>
#include <string>
#include "../TypeReconstruct.h"

#define F_REFLECT_BRICK_PROPERTIES_MOTOR_BRICK (BASE + 0x0C58560)

HOOK(UMotorBrick_ReflectProperties, F_REFLECT_BRICK_PROPERTIES_MOTOR_BRICK, [](SDK::UMotorBrick* This, void *Params) -> void
{

}, void(SDK::UMotorBrick*, void*))