// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Attributes/GPPlayerAttributeSet.h"
#include "Core/Actors/PrototypeBaseCharacter.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

//CSTR
UGPPlayerAttributeSet::UGPPlayerAttributeSet() {}

void UGPPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGPPlayerAttributeSet, Strength);
	DOREPLIFETIME(UGPPlayerAttributeSet, Agility);
	DOREPLIFETIME(UGPPlayerAttributeSet, Vitality);
	DOREPLIFETIME(UGPPlayerAttributeSet, Intelligence);
	DOREPLIFETIME(UGPPlayerAttributeSet, Dexterity);
	DOREPLIFETIME(UGPPlayerAttributeSet, Luck);
	DOREPLIFETIME(UGPPlayerAttributeSet, Precision);
	DOREPLIFETIME(UGPPlayerAttributeSet, Reflex);
	DOREPLIFETIME(UGPPlayerAttributeSet, XP);
}

void UGPPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	//if (Attribute == GetMaxHealthAttribute())
	//{
	//	AdjustAttributeForMaxChanged(Health, MaxHealth, NewValue, GetHealthAttribute());
	//}
}

void UGPPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
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
		if (Data.EvaluatedData.Attribute == GetStrengthAttribute())
		{
			TargetCharacter->HandleStrengthChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetAgilityAttribute())
		{
			TargetCharacter->HandleAgilityChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetVitalityAttribute())
		{
			TargetCharacter->HandleVitalityChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetIntelligenceAttribute())
		{
			TargetCharacter->HandleIntelligenceChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetDexterityAttribute())
		{
			TargetCharacter->HandleDexterityChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetLuckAttribute())
		{
			TargetCharacter->HandleLuckChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetPrecisionAttribute())
		{
			TargetCharacter->HandlePrecisionChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetReflexAttribute())
		{
			TargetCharacter->HandleReflexChanged(DeltaValue, SourceTags);
		}

		if (Data.EvaluatedData.Attribute == GetXPAttribute())
		{
			TargetCharacter->HandleXPChanged(DeltaValue, SourceTags);
		}
	}
}

void UGPPlayerAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPPlayerAttributeSet, Strength, OldValue);
}

void UGPPlayerAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPPlayerAttributeSet, Agility, OldValue);
}

void UGPPlayerAttributeSet::OnRep_Vitality(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPPlayerAttributeSet, Vitality, OldValue);
}

void UGPPlayerAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPPlayerAttributeSet, Intelligence, OldValue);
}

void UGPPlayerAttributeSet::OnRep_Dexterity(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPPlayerAttributeSet, Dexterity, OldValue);
}

void UGPPlayerAttributeSet::OnRep_Luck(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPPlayerAttributeSet, Luck, OldValue);
}

void UGPPlayerAttributeSet::OnRep_Precision(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPPlayerAttributeSet, Precision, OldValue);
}

void UGPPlayerAttributeSet::OnRep_Reflex(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPPlayerAttributeSet, Reflex, OldValue);
}

void UGPPlayerAttributeSet::OnRep_XP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPPlayerAttributeSet, XP, OldValue);
}
