// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Abilities/PrototypeGameplayAbility.h"
#include "AbilitySystemLog.h"
#include "AbilitySystemGlobals.h"
#include "Core/Abilities/GPAbilitySystemGlobals.h"
#include "Core/Attributes/PrototypeAttributeSet.h"
#include "Core/Actors/PrototypeBaseCharacter.h"
#include "Core/Components/PrototypeAbilitySystemComponent.h"
#include "BlueprintGameplayTagLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PrototypeGameplayAbility)

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
	if (!ensure(IsInstantiated()))																														\
	{																																					\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
		return ReturnValue;																																\
	}																																					\
}

UPrototypeGameplayAbility::UPrototypeGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActiveCameraMode = nullptr;
}

TSubclassOf<UGameplayEffect> UPrototypeGameplayAbility::GetAttackWeightGameplayEffect() const
{
	return AttackWeightGameplayEffectClass;
}

float UPrototypeGameplayAbility::GetAbilityAttackWeight() const
{
	return AttackWeight.Magnitude;
}

void UPrototypeGameplayAbility::SetCameraMode(TSubclassOf<UGPCameraMode> CameraMode)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(SetCameraMode, );
	if (APrototypeBaseCharacter* OwnerActor = Cast<APrototypeBaseCharacter>(GetActorInfo().AvatarActor))
	{
		OwnerActor->SetAbilityCameraMode(CameraMode, CurrentSpecHandle);
		ActiveCameraMode = CameraMode;
	}
}

void UPrototypeGameplayAbility::ClearCameraMode()
{

	if (ActiveCameraMode)
	{
		if (APrototypeBaseCharacter* OwnerActor = Cast<APrototypeBaseCharacter>(GetActorInfo().AvatarActor))
		{
			OwnerActor->ClearAbilityCameraMode(CurrentSpecHandle);
		}

		ActiveCameraMode = nullptr;
	}
}

FGameplayAbilitySpecHandle UPrototypeGameplayAbility::GetSpecHandle()
{
	return GetCurrentAbilitySpecHandle();
}

bool UPrototypeGameplayAbility::CanActivateGameplayAbility(const AActor* TargetActor)
{
	bool bCanActivate = false;
	if (CurrentActorInfo != nullptr)
	{
		const FGameplayAbilitySpecHandle Handle = GetSpecHandle();
		const AActor* OwnerActor = CurrentActorInfo->OwnerActor.Get();
		UPrototypeAbilitySystemComponent* OwnerASC = (OwnerActor != nullptr) ? OwnerActor->GetComponentByClass<UPrototypeAbilitySystemComponent>() : nullptr;

		bCanActivate = CanActivateAbility(Handle, CurrentActorInfo);
		if (OwnerASC != nullptr)
		{
			bCanActivate = bCanActivate && OwnerASC->CanApplyAttackWeight(this, TargetActor);
		}
	}

	return bCanActivate;
}