// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Abilities/GPGameplayAbilitySet.h"
#include "Core/Actors/PrototypeBaseCharacter.h"
#include "Core/Abilities/PrototypeGameplayAbility.h"
#include "Core/GPInputConfig.h"
#include "Core/Components/PrototypeAbilitySystemComponent.h"
#include "GPLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GPGameplayAbilitySet)

void FGPAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FGPAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FGPAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);
}

void FGPAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* TargetASC)
{
	check(TargetASC);

	if (!TargetASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			TargetASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			TargetASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		TargetASC->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

UGPGameplayAbilitySet::UGPGameplayAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGPGameplayAbilitySet::K2_GiveToAbilitySystem(UAbilitySystemComponent* TargetASC, FGPAbilitySet_GrantedHandles& GrantedHandlesOUT) const
{
	GiveToAbilitySystem(TargetASC, &GrantedHandlesOUT);
}

void UGPGameplayAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* TargetASC, FGPAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(TargetASC);

	if (!TargetASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	// Grant the gameplay abilities.
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FGPAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogGPAbilitySystem, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}

		UPrototypeGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UPrototypeGameplayAbility>();

		APrototypeBaseCharacter* actor = Cast<APrototypeBaseCharacter>(TargetASC->AbilityActorInfo->OwnerActor);
		UGPInputConfig* config = nullptr;
		if (actor)
		{
			config = actor->InputConfig;
		}

		if (config)
		{
			TSubclassOf<UPrototypeGameplayAbility> AbilityClass = AbilityCDO->GetClass();
			const FGameplayTag* inputTag = config->FindGameplayTagForAbility(AbilityClass, true);
			AbilityCDO->AbilityInputID = *config->FindGameplayTagForAbility(AbilityClass, true);
		}
		else
		{
			AbilityCDO->AbilityInputID = AbilityToGrant.InputTag;
		}


		FGameplayAbilitySpec AbilitySpec(
			AbilityCDO, 
			AbilityToGrant.AbilityLevel,
			GetTypeHash(AbilityCDO->AbilityInputID),
			SourceObject
			);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = TargetASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}

	// Grant the gameplay effects.
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FGPAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOG(LogGPAbilitySystem, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = TargetASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, TargetASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}

	// Grant the attribute sets.
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FGPAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			UE_LOG(LogGPAbilitySystem, Error, TEXT("GrantedAttributes[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(TargetASC->GetOwner(), SetToGrant.AttributeSet);
		TargetASC->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}
}

void UGPGameplayAbilitySet::TakeFromAbilitySystem(UAbilitySystemComponent* TargetASC)
{
	GrantedHandles.TakeFromAbilitySystem(TargetASC);
}

void UGPGameplayAbilitySet::AddAbilitySpecHandle(FGameplayAbilitySpecHandle AbilitySpecHandle)
{
	GrantedHandles.AddAbilitySpecHandle(AbilitySpecHandle);
}
