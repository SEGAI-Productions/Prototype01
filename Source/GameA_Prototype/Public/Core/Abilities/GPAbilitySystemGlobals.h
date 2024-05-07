// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "GPAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UGPAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
    UGPAbilitySystemGlobals();

	/** TryActivate failed due to not being able to spend AttackWeights */
	UPROPERTY()
	FGameplayTag ActivateFailAttackWeightTag;
	UPROPERTY(config)
	FName ActivateFailAttackWeightName;

	/** Initialize global tags by reading from config using the names and creating tags for use at runtime */
	virtual void InitGlobalTags()
	{
		Super::InitGlobalTags();

		if (ActivateFailAttackWeightName != NAME_None)
		{
			ActivateFailAttackWeightTag = FGameplayTag::RequestGameplayTag(ActivateFailAttackWeightName);
		}
	}
};
