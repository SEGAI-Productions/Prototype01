// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/PrototypeData.h"
#include "GPInputConfig.generated.h"

class UInputAction;
struct FGameplayTag;
class UPrototypeGameplayAbility;

/**
 * FTaggedInputAction
 *
 *	Struct used to map an input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FTaggedInputAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	TSubclassOf<UPrototypeGameplayAbility> GameplayAbility;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UGPInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Returns the first Input Action associated with a given tag.
	const UInputAction* FindInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const;

	// Returns the first Input Action associated with a given ability.
	const FGameplayTag* FindGameplayTagForAbility(TSubclassOf<UPrototypeGameplayAbility>& Ability, bool bLogNotFound) const;

	// List of input actions used by the owner. These input actions are mapped to a gameplay tag and must be manually bound.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FTaggedInputAction> TaggedInputActions;
};
