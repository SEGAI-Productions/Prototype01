// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Core/Attributes/PrototypeAttributeSet.h"
#include "GPPlayerAttributeSet.generated.h"

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
class GAMEA_PROTOTYPE_API UGPPlayerAttributeSet : public UPrototypeAttributeSet
{
	GENERATED_BODY()

public:
	UGPPlayerAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called whenever an effect modifies an attribute.
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// Called every time a gameplay effect is applied to the ability system that controls this attribute set.
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UFUNCTION()
	virtual void OnRep_Strength(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Agility(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Vitality(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Intelligence(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Dexterity(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Luck(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Precision(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Reflex(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_XP(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UGPPlayerAttributeSet, Strength);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Agility, Category = "Attributes")
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS(UGPPlayerAttributeSet, Agility);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vitality, Category = "Attributes")
	FGameplayAttributeData Vitality;
	ATTRIBUTE_ACCESSORS(UGPPlayerAttributeSet, Vitality);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UGPPlayerAttributeSet, Intelligence);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Dexterity, Category = "Attributes")
	FGameplayAttributeData Dexterity;
	ATTRIBUTE_ACCESSORS(UGPPlayerAttributeSet, Dexterity);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Luck, Category = "Attributes")
	FGameplayAttributeData Luck;
	ATTRIBUTE_ACCESSORS(UGPPlayerAttributeSet, Luck);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Precision, Category = "Attributes")
	FGameplayAttributeData Precision;
	ATTRIBUTE_ACCESSORS(UGPPlayerAttributeSet, Precision);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Reflex, Category = "Attributes")
	FGameplayAttributeData Reflex;
	ATTRIBUTE_ACCESSORS(UGPPlayerAttributeSet, Reflex);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_XP, Category = "Attributes")
	FGameplayAttributeData XP;
	ATTRIBUTE_ACCESSORS(UGPPlayerAttributeSet, XP);
	
};
