// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "PrototypeAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReceiedDamageDelegate, UPrototypeAbilitySystemComponent*, SourceASC, float, UnmitigatedDamage, float, MitigatedDamage);

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UPrototypeAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	UPrototypeAbilitySystemComponent();

	void AbilityInputTagPressed(int32 InputID);
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	FReceiedDamageDelegate ReceivedDamage;

	virtual void ReceiveDamage(UPrototypeAbilitySystemComponent* SourceASC, float UnmitigatedDamage, float MitigatedDamage);

	/** Perform a preliminary check if the ability's attack weight can be applied. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	bool CanApplyAttckWeight(UPrototypeGameplayAbility* Ability, UAbilitySystemComponent* TargetASC) const;

	/** Perform a preliminary check if the ability's attack weight can be applied. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	bool TryApplyAttackWeight(UPrototypeGameplayAbility* Ability, UAbilitySystemComponent* TargetASC) const;

protected:

	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
};
