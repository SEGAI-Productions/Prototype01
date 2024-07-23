// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Camera/SegaiCameraManager.h"

#include "Async/TaskGraphInterfaces.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Core/Camera/GPCameraComponent.h"
#include "Core/Camera/SegaiUICameraManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SegaiCameraManager)

class FDebugDisplayInfo;

static FName UICameraComponentName(TEXT("UICamera"));

ASegaiCameraManager::ASegaiCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultFOV = GP_CAMERA_DEFAULT_FOV;
	ViewPitchMin = GP_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = GP_CAMERA_DEFAULT_PITCH_MAX;

	UICamera = CreateDefaultSubobject<USegaiUICameraManagerComponent>(UICameraComponentName);
}

USegaiUICameraManagerComponent* ASegaiCameraManager::GetUICameraComponent() const
{
	return UICamera;
}

void ASegaiCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// If the UI Camera is looking at something, let it have priority.
	if (UICamera->NeedsToUpdateViewTarget())
	{
		Super::UpdateViewTarget(OutVT, DeltaTime);
		UICamera->UpdateViewTarget(OutVT, DeltaTime);
		return;
	}

	Super::UpdateViewTarget(OutVT, DeltaTime);
}

void ASegaiCameraManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("SegaiCameraManager: %s"), *GetNameSafe(this)));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	const APawn* Pawn = (PCOwner ? PCOwner->GetPawn() : nullptr);

	if (const UGPCameraComponent* CameraComponent = UGPCameraComponent::FindCameraComponent(Pawn))
	{
		CameraComponent->DrawDebug(Canvas);
	}
}
