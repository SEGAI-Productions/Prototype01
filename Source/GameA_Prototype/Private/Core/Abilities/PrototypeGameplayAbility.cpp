// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Abilities/PrototypeGameplayAbility.h"

#include "AbilitySystemLog.h"
#include "Curves/CurveVector.h"
#include "AbilitySystemGlobals.h"
#include "BlueprintGameplayTagLibrary.h"
#include "Core/Camera/GPCameraComponent.h"
#include "Core/Subsystems/CooldownSubsystem.h"
#include "Core/Actors/PrototypeBaseCharacter.h"
#include "Core/Abilities/GPAbilitySystemGlobals.h"
#include "Core/Attributes/PrototypeAttributeSet.h"
#include "Core/Components/PrototypeAbilitySystemComponent.h"

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

void UPrototypeGameplayAbility::SetDynamicOffsetCurve(UCurveVector* DynamicOffset)
{
	if (UGPCameraComponent* CameraComponent = GetOwnerCameraComponent().Get())
	{
		CameraComponent->SetDynamicOffsetCurve(GetActiveCameraMode(), DynamicOffset);
	}
}

void UPrototypeGameplayAbility::FocusActor(AActor* InFocusActor)
{
	if (UGPCameraComponent* CameraComponent = GetOwnerCameraComponent().Get())
	{
		CameraComponent->SetFocusObject(GetActiveCameraMode(), InFocusActor);
	}
}

TSubclassOf<UGPCameraMode> UPrototypeGameplayAbility::GetActiveCameraMode()
{
	return ActiveCameraMode;
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

bool UPrototypeGameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bNoCooldown = Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);

	if (bNoCooldown && (GlobalCooldownDuration > 0 || CooldownDuration > 0))
	{
		UWorld* World = GetWorld();
		UGameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance() : nullptr;
		UCooldownSubsystem* CooldownSubsystem = (GameInstance != nullptr) ? GameInstance->GetSubsystem<UCooldownSubsystem>() : nullptr;
		AActor* OwnerActor = CurrentActorInfo->OwnerActor.Get();
		if (CooldownSubsystem && OwnerActor)
		{
			bNoCooldown = !CooldownSubsystem->IsOnCooldown(GetClass(), OwnerActor);
		}
	}

	return bNoCooldown;
}

void UPrototypeGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance() : nullptr;
	UCooldownSubsystem* CooldownSubsystem = (GameInstance != nullptr) ? GameInstance->GetSubsystem<UCooldownSubsystem>() : nullptr;
	AActor* OwnerActor = CurrentActorInfo->OwnerActor.Get();
	if (CooldownSubsystem && OwnerActor)
	{
		if (GlobalCooldownDuration > 0)
		{
			CooldownSubsystem->ApplyGlobalCooldown(GetClass(), OwnerActor, GlobalCooldownDuration);
		}

		if (CooldownDuration > 0)
		{
			CooldownSubsystem->ApplyCooldown(GetClass(), OwnerActor, CooldownDuration);
		}
	}
}

TObjectPtr<UGPCameraComponent> UPrototypeGameplayAbility::GetOwnerCameraComponent()
{
	FGameplayAbilityActorInfo ActorInfo = GetActorInfo();
	AActor* Actor = ActorInfo.AvatarActor.Get();

	if (APrototypeBaseCharacter* Character = Cast<APrototypeBaseCharacter>(Actor))
	{
		return Character->CameraComponent;
	}

	return nullptr;
}
