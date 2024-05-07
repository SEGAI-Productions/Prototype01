// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Components/GPEnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Core/Actors/PrototypeBaseCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GPEnhancedInputComponent)

class UGPInputConfig;

UGPEnhancedInputComponent::UGPEnhancedInputComponent(const FObjectInitializer& ObjectInitializer) {}

void UGPEnhancedInputComponent::AddInputMappings(const UGPInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UGPEnhancedInputComponent::RemoveInputMappings(const UGPInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UGPEnhancedInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}