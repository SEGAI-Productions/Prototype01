// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Camera/GPCameraMode_Dynamic.h"
#include "Core/Camera/GPCameraMode.h"
#include "Components/PrimitiveComponent.h"
#include "Curves/CurveVector.h"
#include "Engine/Canvas.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/RotationMatrix.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GPCameraMode_Dynamic)

UGPCameraMode_Dynamic::UGPCameraMode_Dynamic()
{
	TargetOffsetCurve = nullptr;
	DynamicOffsetCurve = nullptr;
}

void UGPCameraMode_Dynamic::UpdateView(float DeltaTime)
{
	Super::UpdateView(DeltaTime);

	// Get the target actor and ensure it's valid.
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	// Set initial view properties.
	CurrentFOVOffset = View.Location;
	float AngleBetween = 0.0f;

	// Get the current pivot rotation and location.
	FVector PivotLocation = GetPivotLocation() + CurrentCrouchOffset;
	FVector LookAtLocation = PivotLocation;
	FRotator PivotRotation = View.ControlRotation;

	// Determine the focus location based on the focus actor or the target actor.
	if (FocusActor)
	{
		// Clamp the pitch of the pivot rotation within specified limits.
		PivotRotation.Pitch = 0.0f;

		// Apply Dynamic offset using ElapsedTime.
		if (DynamicOffsetCurve)
		{
			FVector DynamicOffset = DynamicOffsetCurve->GetVectorValue(ElapsedTime) + TargetOffset;
			View.Location = PivotLocation + PivotRotation.RotateVector(DynamicOffset);
			ElapsedTime += DeltaTime;
			//UE_LOG(LogTemp, Warning, TEXT("PivotLocation = X: %f, Y: %f, Z: %f"), PivotLocation.X, PivotLocation.Y, PivotLocation.Z);
			//UE_LOG(LogTemp, Warning, TEXT("View.Location = X: %f, Y: %f, Z: %f"), View.Location.X, View.Location.Y, View.Location.Z);
		}

		UpdateLookAtRotation(FocusActor, PivotLocation);
		AdjustCameraIfNecessary(PivotLocation, GetFocusActorLocation(FocusActor), DeltaTime);
	}
	else if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// If the target actor is a pawn, get its focus location from a socket if it's a character.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			if (TargetCharacter->GetMesh()->DoesSocketExist(FocusSocketName))
			{
				LookAtLocation = TargetCharacter->GetMesh()->GetSocketLocation(FocusSocketName);

				// Calculate the rotation vector and set the view rotation accordingly
				FVector RotationVector = LookAtLocation - View.Location;
				FRotator ViewRotation = RotationVector.Rotation();
				View.Rotation = ViewRotation;
			}
		}
	}

	//UE_LOG(LogTemp, Warning, TEXT("View.Rotation =  Pitch: %f, Yaw: %f"), View.Rotation.Pitch, View.Rotation.Yaw);

	// Adjust final desired camera location to prevent any penetration
	UpdatePreventPenetration(DeltaTime);
}

void UGPCameraMode_Dynamic::OnActivation()
{
}

void UGPCameraMode_Dynamic::OnDeactivation()
{
	ElapsedTime = 0.0f;
	SetDynamicOffsetCurve(nullptr);
	FocusActor = nullptr;
}

void UGPCameraMode_Dynamic::SetDynamicOffsetCurve(TObjectPtr<const UCurveVector> DynamicOffset)
{
	DynamicOffsetCurve = DynamicOffset;
}
