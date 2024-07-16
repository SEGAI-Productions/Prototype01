// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shakes/LegacyCameraShake.h"
#include "GPImpactCameraShake.generated.h"

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UGPImpactCameraShake : public ULegacyCameraShake
{
	GENERATED_BODY()
	
public:
	UGPImpactCameraShake();

	UFUNCTION(BlueprintCallable, Category = "OscillationSettings")
	void SetOscillationDuration(float Duration);
};
