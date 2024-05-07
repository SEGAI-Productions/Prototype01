// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnhancedInputComponent.h"
#include "Core/GPInputConfig.h"

#include "GPEnhancedInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;

/**
 * GPInputComponent
 *
 *	Component used to manage input mappings and bindings using an input config data asset.
 */
UCLASS(Config = Input)
class GAMEA_PROTOTYPE_API UGPEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UGPEnhancedInputComponent(const FObjectInitializer& ObjectInitializer);

	void AddInputMappings(const UGPInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	void RemoveInputMappings(const UGPInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	void RemoveBinds(TArray<uint32>& BindHandles);
};
