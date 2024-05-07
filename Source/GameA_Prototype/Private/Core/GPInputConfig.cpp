// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GPInputConfig.h"
#include "Core/PrototypeData.h"
#include "GPLogChannels.h"
#include "Core/Abilities/PrototypeGameplayAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GPInputConfig)

const UInputAction* UGPInputConfig::FindInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FTaggedInputAction& TaggedInputAction : TaggedInputActions)
	{
		if (TaggedInputAction.InputAction && TaggedInputAction.InputTag == InputTag)
		{
			return TaggedInputAction.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogGP, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const FGameplayTag* UGPInputConfig::FindGameplayTagForAbility(TSubclassOf<UPrototypeGameplayAbility>& Ability, bool bLogNotFound) const
{
	for (const FTaggedInputAction& TaggedInputAction : TaggedInputActions)
	{
		if (TaggedInputAction.GameplayAbility && TaggedInputAction.GameplayAbility.GetDefaultObject()->GetFName() == Ability.GetDefaultObject()->GetFName())
		{
			UE_LOG(LogGP, Warning, TEXT("Comparing Tagged Ability [%s] to Ability [%s]."), *TaggedInputAction.GameplayAbility.GetDefaultObject()->GetFName().ToString(), *Ability.GetDefaultObject()->GetFName().ToString());
			UE_LOG(LogGP, Warning, TEXT("Returning Tag [%s]"), *TaggedInputAction.InputTag.ToString());
			return &TaggedInputAction.InputTag;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogGP, Error, TEXT("Can't find AbilityInputAction for Abilty [%s] on InputConfig [%s]."), *Ability.GetDefaultObject()->GetFName().ToString(), *GetNameSafe(this));
	}

	return nullptr;
}