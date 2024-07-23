// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Camera/SegaiUICameraManagerComponent.h"

#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "Core/Camera/SegaiCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SegaiUICameraManagerComponent)

class AActor;
class FDebugDisplayInfo;

USegaiUICameraManagerComponent* USegaiUICameraManagerComponent::GetComponent(APlayerController* PC)
{
	if (PC != nullptr)
	{
		if (ASegaiCameraManager* PCCamera = Cast<ASegaiCameraManager>(PC->PlayerCameraManager))
		{
			return PCCamera->GetUICameraComponent();
		}
	}

	return nullptr;
}

// Sets default values for this component's properties
USegaiUICameraManagerComponent::USegaiUICameraManagerComponent()
{
	bWantsInitializeComponent = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// Register "showdebug" hook.
		if (!IsRunningDedicatedServer())
		{
			AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);
		}
	}
}

void USegaiUICameraManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void USegaiUICameraManagerComponent::SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams)
{
	TGuardValue<bool> UpdatingViewTargetGuard(bUpdatingViewTarget, true);

	ViewTarget = InViewTarget;
	CastChecked<ASegaiCameraManager>(GetOwner())->SetViewTarget(ViewTarget, TransitionParams);
}

bool USegaiUICameraManagerComponent::NeedsToUpdateViewTarget() const
{
	return false;
}

void USegaiUICameraManagerComponent::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
}

void USegaiUICameraManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
}