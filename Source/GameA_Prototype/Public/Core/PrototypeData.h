#pragma once

#include "PrototypeData.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None,
	Confirm,
	Cancel,
	Punch,
	Kick_Knee,
	Kick_Front,
	Fire,
	Jump,
	Combo_P_P,
	Combo_P_P_P,
	Combo_P_P_K
};