// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PrototypeData.h"
#include "Abilities/GameplayAbility.h"
#include "Core/Camera/GPCameraMode.h"
#include "PrototypeGameplayAbility.generated.h"

USTRUCT(BlueprintType, meta = (FullyExpand = true))
struct GAMEA_PROTOTYPE_API FCallerMagnitude
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Attack Weight")
	float Magnitude;

	UPROPERTY(EditDefaultsOnly, Category = "Attack Weight")
	FGameplayTag Tag;
};
/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UPrototypeGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPrototypeGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Returns the gameplay effect used to apply cost */
	TSubclassOf<UGameplayEffect> GetAttackWeightGameplayEffect() const;

	/** Apply chosen ability weight to target. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	float GetAbilityAttackWeight() const;

	// Sets the ability's camera mode.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void SetCameraMode(TSubclassOf<UGPCameraMode> CameraMode);

	// Clears the ability's camera mode.  Automatically called if needed when the ability ends.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ClearCameraMode();

	UFUNCTION(BlueprintCallable, Category = "Ability")
	FGameplayAbilitySpecHandle GetSpecHandle();

	// Abilities will activate when input is pressed
	FGameplayTag AbilityInputID;

	// Current camera mode set by the ability.
	TSubclassOf<UGPCameraMode> ActiveCameraMode;

	/** This GameplayEffect represents the attack weight of the ability. It will be applied when the ability is committed. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack Weight")
	TSubclassOf<UGameplayEffect> AttackWeightGameplayEffectClass;

	/** This GameplayEffect represents the attack weight of the ability. It will be applied when the ability is committed. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack Weight")
	FCallerMagnitude AttackWeight;

	/** This GameplayEffect represents the attack weight of the ability. It will be applied when the ability is committed. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack Weight")
	FCallerMagnitude AttackDuration;
};
