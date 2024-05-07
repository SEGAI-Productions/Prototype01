// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PrototypeAttributeSet.generated.h"

// Uses macros from Attributes.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UPrototypeAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPrototypeAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called whenever an effect modifies an attribute.
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// Called every time a gameplay effect is applied to the ability system that controls this attribute set.
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackWeight, Category = "Attributes")
	FGameplayAttributeData AttackWeight;
	ATTRIBUTE_ACCESSORS(UPrototypeAttributeSet, AttackWeight);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UPrototypeAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UPrototypeAttributeSet, MaxHealth);

	// Meta Attribute is temporary, do not replicate.
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UPrototypeAttributeSet, Damage);

	void AdjustAttributeForMaxChanged(
		const FGameplayAttributeData& AffectedAttribute,
		const FGameplayAttributeData& MaxAttribute,
		float NewMaxValue,
		const FGameplayAttribute& AffectdAttributeProperty) const;

	UFUNCTION()
	virtual void OnRep_AttackWeight(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
};
