// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Animation/GPAnimInstance.h"
#include "AbilitySystemGlobals.h"
#include "Core/Actors/PrototypeBaseCharacter.h"
#include "Core/Components/GPCharacterMovementComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GPAnimInstance)

UGPAnimInstance::UGPAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGPAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this, ASC);
}

#if WITH_EDITOR
EDataValidationResult UGPAnimInstance::IsDataValid(FDataValidationContext& Context) const
{
	Super::IsDataValid(Context);

	GameplayTagPropertyMap.IsDataValid(this, Context);

	return ((Context.GetNumErrors() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}
#endif

void UGPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UGPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const APrototypeBaseCharacter* Character = Cast<APrototypeBaseCharacter>(GetOwningActor());

	if (!Character) return;

	UGPCharacterMovementComponent* CharMoveComp = Cast<UGPCharacterMovementComponent>(Character->GetCharacterMovement());

	if (CharMoveComp == nullptr) return;

	const FGPCharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
	GroundDistance = GroundInfo.GroundDistance;
}
