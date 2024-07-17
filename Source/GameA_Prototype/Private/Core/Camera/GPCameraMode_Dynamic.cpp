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
	float MaxAngle = 40;
	// Get the target actor and ensure it's valid.
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	// Get the current pivot rotation and location.
	FRotator PivotRotation = GetPivotRotation();
	FVector PivotLocation = GetPivotLocation() + CurrentCrouchOffset;

	// Update target and crouch offsets.
	UpdateForTarget(DeltaTime);
	UpdateCrouchOffset(DeltaTime);

	// Clamp the pitch of the pivot rotation within specified limits.
	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	// Set initial view properties.
	CurrentFOVOffset = View.Location;
	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = PivotRotation;
	View.FieldOfView = FieldOfView;
	FVector FocusLocation = PivotLocation;
	float AngleBetween = 0.0f;

	// Initialize the last updated focus location if it's zero.
	if (LastUpdatedFocusLocation == FVector::ZeroVector)
	{
		LastUpdatedFocusLocation = PivotLocation;
	}

	FVector TargetOffset = PivotLocation;

	// If runtime float curves are not used, update the camera location based on focus and target offsets.
	if (!bUseRuntimeFloatCurves)
	{
		if (TargetOffsetCurve)
		{
			// Apply Thirdperson offset using pitch.
			TargetOffset = TargetOffsetCurve->GetVectorValue(PivotRotation.Pitch);
			View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);
		}

	}
	else
	{
		TargetOffset.X = TargetOffsetX.GetRichCurveConst()->Eval(PivotRotation.Pitch);
		TargetOffset.Y = TargetOffsetY.GetRichCurveConst()->Eval(PivotRotation.Pitch);
		TargetOffset.Z = TargetOffsetZ.GetRichCurveConst()->Eval(PivotRotation.Pitch);

		TargetOffset = PivotLocation + PivotRotation.RotateVector(TargetOffset);
	}


	// Determine the focus location based on the focus actor or the target actor.
	if (FocusActor)
	{
		// Clamp the pitch of the pivot rotation within specified limits.
		PivotRotation.Pitch = 0.0f;

		// Apply Dynamic offset using ElapsedTime.
		if (DynamicOffsetCurve)
		{
			FVector FocusOffset = DynamicOffsetCurve->GetVectorValue(ElapsedTime) + TargetOffset;
			View.Location = PivotLocation + PivotRotation.RotateVector(FocusOffset);
			ElapsedTime += DeltaTime;
			//UE_LOG(LogTemp, Warning, TEXT("PivotLocation = X: %f, Y: %f, Z: %f"), PivotLocation.X, PivotLocation.Y, PivotLocation.Z);
			//UE_LOG(LogTemp, Warning, TEXT("View.Location = X: %f, Y: %f, Z: %f"), View.Location.X, View.Location.Y, View.Location.Z);
		}

		APawn* EnemyPawn = Cast<APawn>(FocusActor);
		ensure(EnemyPawn);

		FVector EnemyLocation = EnemyPawn->GetActorLocation();

		FocusLocation = (PivotLocation + EnemyLocation) / 2;

		// Calculate the rotation vector and set the view rotation accordingly
		FVector RotationVector = FocusLocation - View.Location;
		FRotator ViewRotation = RotationVector.Rotation();
		View.Rotation = ViewRotation;

		AdjustCameraIfNecessary(PivotLocation, EnemyLocation, DeltaTime);
	}
	else if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// If the target actor is a pawn, get its focus location from a socket if it's a character.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			if (TargetCharacter->GetMesh()->DoesSocketExist(FocusSocketName))
			{
				FocusLocation = TargetCharacter->GetMesh()->GetSocketLocation(FocusSocketName);

				// Calculate the rotation vector and set the view rotation accordingly
				FVector RotationVector = FocusLocation - View.Location;
				FRotator ViewRotation = RotationVector.Rotation();
				View.Rotation = ViewRotation;
			}
		}
	}

	// Update the last known focus location
	LastUpdatedFocusLocation = FocusLocation;

	//UE_LOG(LogTemp, Warning, TEXT("View.Rotation =  Pitch: %f, Yaw: %f"), View.Rotation.Pitch, View.Rotation.Yaw);

	// Adjust final desired camera location to prevent any penetration
	UpdatePreventPenetration(DeltaTime);
}

// Function to adjust the camera position if both the player and enemy are not in FOV
void UGPCameraMode_Dynamic::AdjustCameraIfNecessary(FVector PlayerLocation, FVector EnemyLocation, float DeltaTime)
{
	bool bIsPlayerInFOV = IsActorInFOV(PlayerLocation);
	bool bIsEnemyInFOV = IsActorInFOV(EnemyLocation);

	if (!bIsPlayerInFOV || !bIsEnemyInFOV)
	{
		// Calculate the necessary distance to move back to have both actors in FOV
		float DistanceToMoveBack = CalculateDistanceToMoveBack(PlayerLocation, EnemyLocation, View.FieldOfView / 2);

		// Calculate the view direction and new camera position
		FVector ViewForward = View.Rotation.Vector();
		ViewForward.Normalize();

		// Adjust camera position by moving back
		FVector NewViewLocation = View.Location - (ViewForward * DistanceToMoveBack);

		// Smooth out the camera transition
		View.Location = FMath::VInterpTo(CurrentFOVOffset, NewViewLocation, DeltaTime, InterpolationSpeed);
	}
}

// Function to calculate the distance to move the camera back to have both actors in FOV
float UGPCameraMode_Dynamic::CalculateDistanceToMoveBack(const FVector& PlayerLocation, const FVector& EnemyLocation, float FOVAngle)
{
	FVector MidPoint = (PlayerLocation + EnemyLocation) / 2;
	float HalfFOV = FOVAngle / 2.0f;

	// Calculate the distance between the player and enemy
	float DistanceBetweenActors = FVector::Dist(PlayerLocation, EnemyLocation);

	// Calculate the minimum distance to move back based on the FOV
	float DistanceToMoveBack = (DistanceBetweenActors / 2.0f) / FMath::Tan(FMath::DegreesToRadians(HalfFOV));

	return DistanceToMoveBack;
}


// Function to check if an actor is within the camera's FOV
bool UGPCameraMode_Dynamic::IsActorInFOV(FVector ActorLocation)
{
	FVector Direction = ActorLocation - View.Location;
	Direction.Normalize();

	FVector ForwardVector = View.Rotation.Vector();

	float DotProduct = FVector::DotProduct(ForwardVector, Direction);
	float FOV = View.FieldOfView / 2;

	float Angle = FMath::Acos(DotProduct) * (180.0f / PI);
	return Angle <= (FOV / 2.0f);
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

void UGPCameraMode_Dynamic::UpdateForTarget(float DeltaTime)
{
	if (const ACharacter* TargetCharacter = Cast<ACharacter>(GetTargetActor()))
	{
		if (TargetCharacter->bIsCrouched)
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			const float CrouchedHeightAdjustment = TargetCharacterCDO->CrouchedEyeHeight - TargetCharacterCDO->BaseEyeHeight;

			SetTargetCrouchOffset(FVector(0.f, 0.f, CrouchedHeightAdjustment));

			return;
		}
	}

	SetTargetCrouchOffset(FVector::ZeroVector);
}

void UGPCameraMode_Dynamic::UpdatePreventPenetration(float DeltaTime)
{
}

void UGPCameraMode_Dynamic::PreventCameraPenetration(AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly)
{
}

void UGPCameraMode_Dynamic::SetDynamicOffsetCurve(TObjectPtr<const UCurveVector> DynamicOffset)
{
	DynamicOffsetCurve = DynamicOffset;
}

void UGPCameraMode_Dynamic::DrawDebug(UCanvas* Canvas) const
{
	Super::DrawDebug(Canvas);

#if ENABLE_DRAW_DEBUG
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	for (int i = 0; i < DebugActorsHitDuringCameraPenetration.Num(); i++)
	{
		DisplayDebugManager.DrawString(
			FString::Printf(TEXT("HitActorDuringPenetration[%d]: %s")
				, i
				, *DebugActorsHitDuringCameraPenetration[i]->GetName()));
	}

	LastDrawDebugTime = GetWorld()->GetTimeSeconds();
#endif
}

void UGPCameraMode_Dynamic::SetTargetCrouchOffset(FVector NewTargetOffset)
{
	CrouchOffsetBlendPct = 0.0f;
	InitialCrouchOffset = CurrentCrouchOffset;
	TargetCrouchOffset = NewTargetOffset;
}

void UGPCameraMode_Dynamic::UpdateCrouchOffset(float DeltaTime)
{
	if (CrouchOffsetBlendPct < 1.0f)
	{
		CrouchOffsetBlendPct = FMath::Min(CrouchOffsetBlendPct + DeltaTime * CrouchOffsetBlendMultiplier, 1.0f);
		CurrentCrouchOffset = FMath::InterpEaseInOut(InitialCrouchOffset, TargetCrouchOffset, CrouchOffsetBlendPct, 1.0f);
	}
	else
	{
		CurrentCrouchOffset = TargetCrouchOffset;
		CrouchOffsetBlendPct = 1.0f;
	}
}
