// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Attributes/PrototypeAttributeSet.h"
#include "Core/Actors/PrototypeBaseCharacter.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

//CSTR
UPrototypeAttributeSet::UPrototypeAttributeSet() {}

void UPrototypeAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPrototypeAttributeSet, MaxHealth);
	DOREPLIFETIME(UPrototypeAttributeSet, Health);
	DOREPLIFETIME(UPrototypeAttributeSet, AttackWeight);
}

void UPrototypeAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChanged(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
}

void UPrototypeAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();

	float DeltaValue{ 0.0f };

	if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
	{
		DeltaValue = Data.EvaluatedData.Magnitude;
	}

	APrototypeBaseCharacter* TargetCharacter{ nullptr };

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		AActor* TargetActor{ nullptr };
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetCharacter = Cast<APrototypeBaseCharacter>(TargetActor);
	}

	if (TargetCharacter)
	{
		if (Data.EvaluatedData.Attribute == GetAttackWeightAttribute())
		{
			TargetCharacter->HandleAttackWeightChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
		{
			TargetCharacter->HandleMaxHealthChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetHealthAttribute())
		{
			SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
			TargetCharacter->HandleHealthChanged(DeltaValue, SourceTags);
		}
	}
}

void UPrototypeAttributeSet::AdjustAttributeForMaxChanged(
	const FGameplayAttributeData& AffectedAttribute,
	const FGameplayAttributeData& MaxAttribute,
	float NewMaxValue,
	const FGameplayAttribute& AffectdAttributeProperty) const
{
	UAbilitySystemComponent* AbilityComponent = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();

	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComponent)
	{
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		const float NewDelta = (CurrentMaxValue > 0.0f) ?
			(CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComponent->ApplyModToAttributeUnsafe(AffectdAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UPrototypeAttributeSet::OnRep_AttackWeight(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPrototypeAttributeSet, AttackWeight, OldValue);
}

void UPrototypeAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPrototypeAttributeSet, Health, OldValue);
}

void UPrototypeAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPrototypeAttributeSet, MaxHealth, OldValue);
}