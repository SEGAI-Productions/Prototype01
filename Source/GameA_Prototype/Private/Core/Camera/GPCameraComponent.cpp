// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Camera/GPCameraComponent.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Curves/CurveVector.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Core/Camera/GPCameraMode.h"


UGPCameraComponent::UGPCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CameraModeStack = nullptr;
	FieldOfViewOffset = 0.0f;
}

void UGPCameraComponent::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("GPCameraComponent: %s"), *GetNameSafe(GetTargetActor())));

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Location: %s"), *GetComponentLocation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Rotation: %s"), *GetComponentRotation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   FOV: %f"), FieldOfView));

	check(CameraModeStack);
	CameraModeStack->DrawDebug(Canvas);
}

void UGPCameraComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(/*out*/ OutWeightOfTopLayer, /*out*/ OutTagOfTopLayer);
}

void UGPCameraComponent::SetCameraMode(TSubclassOf<UGPCameraMode> CameraMode)
{
	if (CameraMode) CameraModeStack->PushCameraMode(CameraMode);
}

void UGPCameraComponent::SetFocusObject(TSubclassOf<UGPCameraMode> CameraMode, AActor* NewFocusObject)
{
	if (CameraMode) CameraModeStack->SetFocusObject(CameraMode, NewFocusObject);
}

void UGPCameraComponent::SetFocusObjectList(TSubclassOf<UGPCameraMode> CameraMode, const TArray<AActor*>& NewFocusList)
{
	TArray<TSoftObjectPtr<AActor>> SoftList;

	for (AActor* Actor : NewFocusList)
	{
		if (IsValid(Actor))  // Ensure the actor is valid before converting
		{
			SoftList.Add(Actor);
		}
	}

	if (NewFocusList.Num() > 1)
	{
		if (CameraMode) CameraModeStack->SetFocusList(CameraMode, SoftList);
	}
}

void UGPCameraComponent::SetDynamicOffsetCurve(TSubclassOf<UGPCameraMode> CameraMode, UCurveVector* DynamicOffset)
{
	if (CameraMode) CameraModeStack->SetDynamicOffsetCurve(CameraMode, DynamicOffset);
}

void UGPCameraComponent::FocusSocketByName(TSubclassOf<UGPCameraMode> CameraMode, FName FocusSocket)
{
	if (CameraMode) CameraModeStack->FocusSocketByName(CameraMode, FocusSocket);
}

void UGPCameraComponent::OnRegister()
{
	Super::OnRegister();

	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UGPCameraModeStack>(this);
		check(CameraModeStack);
	}

	CameraModeStack->PushCameraMode(DefaultCameraMode);
}

void UGPCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	check(CameraModeStack);

	UpdateCameraModes();

	FGPCameraModeView CameraModeView;
	CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

	// Keep player controller in sync with the latest view.
	if (APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
	{
		if (APlayerController* PC = TargetPawn->GetController<APlayerController>())
		{
			PC->SetControlRotation(CameraModeView.ControlRotation);
		}
	}

	// Apply any offset that was added to the field of view.
	CameraModeView.FieldOfView += FieldOfViewOffset;
	FieldOfViewOffset = 0.0f;

	// Keep camera component in sync with the latest view.
	SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
	FieldOfView = CameraModeView.FieldOfView;

	// Fill in desired view.
	DesiredView.Location = CameraModeView.Location;
	DesiredView.Rotation = CameraModeView.Rotation;
	DesiredView.FOV = CameraModeView.FieldOfView;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;

	// See if the CameraActor wants to override the PostProcess settings used.
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}


	if (IsXRHeadTrackedCamera())
	{
		// In XR much of the camera behavior above is irrellevant, but the post process settings are not.
		Super::GetCameraView(DeltaTime, DesiredView);
	}
}

void UGPCameraComponent::UpdateCameraModes()
{
	check(CameraModeStack);

	if (CameraModeStack->IsStackActivate())
	{
		if (DetermineCameraModeDelegate.IsBound())
		{
			if (const TSubclassOf<UGPCameraMode> CameraMode = DetermineCameraModeDelegate.Execute())
			{
				CameraModeStack->PushCameraMode(CameraMode);
			}
		}
	}
}
