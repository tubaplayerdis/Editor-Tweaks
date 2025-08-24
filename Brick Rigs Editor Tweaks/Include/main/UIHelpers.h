#pragma once
#include <SDK/BrickRigs_classes.hpp>

bool IsActionNameValid(SDK::UInputActionWidget* Input);
bool IsActionNameValid_N(const SDK::FName* Name);
bool IsColorSame(SDK::FColor Color1, SDK::FColor Color2);
void SelectValidObjects(SDK::UInputActionListWidget* This);
SDK::UInputActionListWidget* GetParentWidget(SDK::UInputActionWidget* Widget);