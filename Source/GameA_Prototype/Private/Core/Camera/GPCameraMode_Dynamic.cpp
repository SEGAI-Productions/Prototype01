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
#include "Math/RotationMatrix.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GPCameraMode_Dynamic)

UGPCameraMode_Dynamic::UGPCameraMode_Dynamic()
{
	TargetOffsetCurve = nullptr;
	DynamicOffsetCurve = nullptr;
}

void UGPCameraMode_Dynamic::UpdateView(float DeltaTime)
{
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
	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = PivotRotation;
	View.FieldOfView = FieldOfView;
	FVector FocusLocation = PivotLocation;
	float AngleBetween = 0.0f;

	// Determine the focus location based on the focus actor or the target actor.
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PlayerController->GetPawn())
		{
			if (FocusActor)
			{
				APawn* EnemyPawn = Cast<APawn>(FocusActor);
				ensure(EnemyPawn);
				FVector PlayerLocation = PlayerPawn->GetActorLocation();
				FVector EnemyLocation = EnemyPawn->GetActorLocation();
				FocusLocation = (PlayerLocation + EnemyLocation) / 2;

				// Calculate the direction vectors from the camera to the player and enemy
				FVector CameraToPlayer = PlayerLocation - View.Location;
				FVector CameraToEnemy = EnemyLocation - View.Location;

				// Normalize the direction vectors
				CameraToPlayer.Normalize();
				CameraToEnemy.Normalize();

				double product = FVector::DotProduct(CameraToPlayer, CameraToEnemy);
				double Acos = FMath::Acos(product);
				AngleBetween = FMath::RadiansToDegrees(Acos);
			}
			else if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
			{
				// If the target actor is a pawn, get its focus location from a socket if it's a character.
				if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
				{
					if (TargetCharacter->GetMesh()->DoesSocketExist(FocusSocketName))
					{
						FocusLocation = TargetCharacter->GetMesh()->GetSocketLocation(FocusSocketName);
					}
				}
			}
		}
	}

	// Initialize the last updated focus location if it's zero.
	if (LastUpdatedFocusLocation == FVector::ZeroVector)
	{
		LastUpdatedFocusLocation = FocusLocation;
	}

	// If runtime float curves are not used, update the camera location based on focus and target offsets.
	if (!bUseRuntimeFloatCurves)
	{
		FVector Offset = FVector::Zero();

		// Apply Dynamic offset using DeltaTime.
		if (DynamicOffsetCurve)
		{
			//FVector FocusOffset = DynamicOffsetCurve->GetVectorValue(ElapsedTime);
			FVector FocusOffset = DynamicOffsetCurve->GetVectorValue(AngleBetween);

			float TimeDilation = UGameplayStatics::GetGlobalTimeDilation(TargetActor->GetWorld());
			float TimeIn = ElapsedTime / TimeDilation;
			FVector FocusOffsetX = DynamicOffsetCurve->GetVectorValue(TimeIn * 10);

			//UE_LOG(LogTemp, Warning, TEXT("ElapsedTime : %f -- TimeDilation : %f -- TimeIn : %f -- FocusOffsetX : %f"), ElapsedTime, TimeDilation, TimeIn, FocusOffsetX.X);

			FocusOffset = FVector(FocusOffsetX.X, FocusOffset.Y, FocusOffset.Z);

			ElapsedTime += DeltaTime;
			Offset = FocusOffset;
			View.Location = Offset;
		}

		// Apply Thirdperson offset using pitch.
		if (TargetOffsetCurve)
		{
			Offset += TargetOffsetCurve->GetVectorValue(PivotRotation.Pitch);
			const FVector TargetOffset = Offset;
			View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);
		}
	}
	else
	{
		FVector TargetOffset(0.0f);

		TargetOffset.X = TargetOffsetX.GetRichCurveConst()->Eval(PivotRotation.Pitch);
		TargetOffset.Y = TargetOffsetY.GetRichCurveConst()->Eval(PivotRotation.Pitch);
		TargetOffset.Z = TargetOffsetZ.GetRichCurveConst()->Eval(PivotRotation.Pitch);

		View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);
	}

	// Update the last known focus location
	LastUpdatedFocusLocation = FocusLocation;
	
	if (FocusActor)
	{
		// Calculate the rotation vector and set the view rotation accordingly
		FVector RotationVector = FocusLocation - View.Location;
		FRotator ViewRotation = RotationVector.Rotation();
		ViewRotation.Pitch = 0.0f;
		View.Rotation = ViewRotation;
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

FVector UGPCameraMode_Dynamic::GetFocusMidpoint(FVector PlayerLocation, FVector EnemyLocation)
{
	float FOV = View.FieldOfView;

	// Calculate the midpoint between the player and the enemy
	FVector Midpoint = (PlayerLocation + EnemyLocation) / 2;

	// Calculate the direction vectors from the camera to the player and enemy
	FVector CameraToPlayer = PlayerLocation - View.Location;
	FVector CameraToEnemy = EnemyLocation - View.Location;

	// Normalize the direction vectors
	CameraToPlayer.Normalize();
	CameraToEnemy.Normalize();

	// Calculate the angle between the direction vectors
	float AngleBetween = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CameraToPlayer, CameraToEnemy)));

	do
	{
		// Calculate the smallest angle to rotate the camera to fit both characters in view
		float AngleToRotate = (AngleBetween - FOV) / 2.0f;

		// Calculate the rotation axis
		FVector RotationAxis = FVector::CrossProduct(CameraToPlayer, CameraToEnemy).GetSafeNormal();

		// Create the rotation quaternion
		FQuat RotationQuat = FQuat(RotationAxis, FMath::DegreesToRadians(AngleToRotate));

		// Calculate the desired rotation for the camera
		FQuat CurrentRotation = View.Rotation.Quaternion();
		FQuat DesiredRotation = RotationQuat * CurrentRotation;

		// Interpolate the camera rotation towards the desired rotation
		//View.Rotation = FMath::Lerp(CurrentRotation, DesiredRotation, 0.1f).Rotator();

		// Set the camera rotation to the desired rotation
		View.Rotation = DesiredRotation.Rotator();

		// Recalculate the direction vectors after the rotation
		CameraToPlayer = PlayerLocation - View.Location;
		CameraToEnemy = EnemyLocation - View.Location;
		CameraToPlayer.Normalize();
		CameraToEnemy.Normalize();

		// Check if the new rotation encompasses both characters
		AngleBetween = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CameraToPlayer, CameraToEnemy)));

		if (AngleBetween > FOV)
		{
			FOV++;
			View.FieldOfView = FOV;
			// move camera backwards away from character midpoint
		}

	} while (AngleBetween > FOV && FOV < 120);

	// Both characters are within the camera's field of view, so return the midpoint
	return Midpoint;
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
