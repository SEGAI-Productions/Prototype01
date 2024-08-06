// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Camera/GPCameraMode_ThirdPerson.h"
#include "Core/Camera/GPCameraMode.h"
#include "Components/PrimitiveComponent.h"
#include "Curves/CurveVector.h"
#include "Engine/Canvas.h"
#include "Core/Camera/SegaiPenetrationAvoidanceFeeler.h"
#include "Core/Camera/SegaiCameraAssistInterface.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "Math/RotationMatrix.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GPCameraMode_ThirdPerson)

namespace GPCameraMode_ThirdPerson_Statics
{
	static const FName NAME_IgnoreCameraCollision = TEXT("IgnoreCameraCollision");
}

UGPCameraMode_ThirdPerson::UGPCameraMode_ThirdPerson()
{
	TargetOffsetCurve = nullptr;

	PenetrationAvoidanceFeelers.Add(FSegaiPenetrationAvoidanceFeeler(FRotator(+00.0f, +00.0f, 0.0f), 1.00f, 1.00f, 14.f, 0));
	PenetrationAvoidanceFeelers.Add(FSegaiPenetrationAvoidanceFeeler(FRotator(+00.0f, +16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FSegaiPenetrationAvoidanceFeeler(FRotator(+00.0f, -16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FSegaiPenetrationAvoidanceFeeler(FRotator(+00.0f, +32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FSegaiPenetrationAvoidanceFeeler(FRotator(+00.0f, -32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FSegaiPenetrationAvoidanceFeeler(FRotator(+20.0f, +00.0f, 0.0f), 1.00f, 1.00f, 00.f, 4));
	PenetrationAvoidanceFeelers.Add(FSegaiPenetrationAvoidanceFeeler(FRotator(-20.0f, +00.0f, 0.0f), 0.50f, 0.50f, 00.f, 4));
}

void UGPCameraMode_ThirdPerson::UpdateView(float DeltaTime)
{
	UpdateForTarget(DeltaTime);
	UpdateCrouchOffset(DeltaTime);

	FVector PivotLocation = GetPivotLocation() + CurrentCrouchOffset;

	// Make sure to clamp the pitch of the controller
	FRotator PivotRotation = GetPivotRotation();
	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);
	View.ControlRotation = PivotRotation;

	FVector ViewLocation = PivotLocation;
	View.Location = PivotLocation;
	FRotator ViewRotation = PivotRotation;
	View.Rotation = PivotRotation;
	View.FieldOfView = FieldOfView;

#pragma region CameraLag

	UpdateCameraLag(DeltaTime, PivotLocation, PivotRotation);

#pragma endregion

	// Apply third person offset using pitch.
	if (!bUseRuntimeFloatCurves)
	{
		if (TargetOffsetCurve)
		{
			TargetOffset = TargetOffsetCurve->GetVectorValue(PivotRotation.Pitch);

			if (FocusActor)
			{
				TargetOffset.Y += 60.0f * (TargetOffset.Y / TargetOffset.Y);
				TargetOffset.Z += 100.0f * (TargetOffset.Z / TargetOffset.Z);
			}

			ViewLocation = PivotLocation + PivotRotation.RotateVector(TargetOffset);
		}
	}
	else
	{
		TargetOffset.X = TargetOffsetX.GetRichCurveConst()->Eval(PivotRotation.Pitch);
		TargetOffset.Y = TargetOffsetY.GetRichCurveConst()->Eval(PivotRotation.Pitch);
		TargetOffset.Z = TargetOffsetZ.GetRichCurveConst()->Eval(PivotRotation.Pitch);

		ViewLocation = PivotLocation + PivotRotation.RotateVector(TargetOffset);
	}

	View.Location = ViewLocation;

	FVector FocusLocation = GetFocusLocation();
	AdjustCameraIfNecessary(PivotLocation, FocusLocation, ViewLocation, DeltaTime);

	// Adjust final desired camera location to prevent any penetration
	UpdatePreventPenetration(DeltaTime);
}

FVector UGPCameraMode_ThirdPerson::GetFocusLocation()
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	if (FocusActor)
	{
		FVector FocusActorLocation = FocusActor->GetActorLocation();
		return FocusActorLocation;
	}

	if (const APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
	{
		// If the target actor is a pawn, get its forward rotation.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			FRotator CharacterRotation = TargetCharacter->GetActorRotation();
			FVector CharacterForwardVector = CharacterRotation.Vector() * 400;
			FVector ForwardLocation = PivotLocation + CharacterForwardVector;
			return ForwardLocation;
		}
	}

	FVector PivotForwardVector = PivotRotation.Vector() * 400;
	FVector PivotForwardLocation = PivotLocation + PivotForwardVector;
	return PivotForwardLocation;
}

void UGPCameraMode_ThirdPerson::UpdateForTarget(float DeltaTime)
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

void UGPCameraMode_ThirdPerson::UpdatePreventPenetration(float DeltaTime)
{
	if (!bPreventPenetration)
	{
		return;
	}

	AActor* TargetActor = GetTargetActor();

	APawn* TargetPawn = Cast<APawn>(TargetActor);
	AController* TargetController = TargetPawn ? TargetPawn->GetController() : nullptr;
	ISegaiCameraAssistInterface* TargetControllerAssist = Cast<ISegaiCameraAssistInterface>(TargetController);

	ISegaiCameraAssistInterface* TargetActorAssist = Cast<ISegaiCameraAssistInterface>(TargetActor);

	TOptional<AActor*> OptionalPPTarget = TargetActorAssist ? TargetActorAssist->GetCameraPreventPenetrationTarget() : TOptional<AActor*>();
	AActor* PPActor = OptionalPPTarget.IsSet() ? OptionalPPTarget.GetValue() : TargetActor;
	ISegaiCameraAssistInterface* PPActorAssist = OptionalPPTarget.IsSet() ? Cast<ISegaiCameraAssistInterface>(PPActor) : nullptr;

	const UPrimitiveComponent* PPActorRootComponent = Cast<UPrimitiveComponent>(PPActor->GetRootComponent());
	if (PPActorRootComponent)
	{
		// Attempt at picking SafeLocation automatically, so we reduce camera translation when aiming.
		// Our camera is our reticle, so we want to preserve our aim and keep that as steady and smooth as possible.
		// Pick closest point on capsule to our aim line.
		FVector ClosestPointOnLineToCapsuleCenter;
		FVector SafeLocation = PPActor->GetActorLocation();
		FMath::PointDistToLine(SafeLocation, View.Rotation.Vector(), View.Location, ClosestPointOnLineToCapsuleCenter);

		// Adjust Safe distance height to be same as aim line, but within capsule.
		float const PushInDistance = PenetrationAvoidanceFeelers[0].Extent + CollisionPushOutDistance;
		float const MaxHalfHeight = PPActor->GetSimpleCollisionHalfHeight() - PushInDistance;
		SafeLocation.Z = FMath::Clamp(ClosestPointOnLineToCapsuleCenter.Z, SafeLocation.Z - MaxHalfHeight, SafeLocation.Z + MaxHalfHeight);

		float DistanceSqr;
		PPActorRootComponent->GetSquaredDistanceToCollision(ClosestPointOnLineToCapsuleCenter, DistanceSqr, SafeLocation);
		// Push back inside capsule to avoid initial penetration when doing line checks.
		if (PenetrationAvoidanceFeelers.Num() > 0)
		{
			SafeLocation += (SafeLocation - ClosestPointOnLineToCapsuleCenter).GetSafeNormal() * PushInDistance;
		}

		// Then aim line to desired camera position
		bool const bSingleRayPenetrationCheck = !bDoPredictiveAvoidance;
		PreventCameraPenetration(*PPActor, SafeLocation, View.Location, DeltaTime, AimLineToDesiredPosBlockedPct, bSingleRayPenetrationCheck);

		ISegaiCameraAssistInterface* AssistArray[] = { TargetControllerAssist, TargetActorAssist, PPActorAssist };

		if (AimLineToDesiredPosBlockedPct < ReportPenetrationPercent)
		{
			for (ISegaiCameraAssistInterface* Assist : AssistArray)
			{
				if (Assist)
				{
					// camera is too close, tell the assists
					Assist->OnCameraPenetratingTarget();
				}
			}
		}
	}
}

float UGPCameraMode_ThirdPerson::CalculateAngleBetweenVectors(const FVector& A, const FVector& B)
{
	float DotProd = FVector::DotProduct(A, B);
	float MagnitudeA = A.Size();
	float MagnitudeB = B.Size();
	float CosTheta = DotProd / (MagnitudeA * MagnitudeB);
	return FMath::Acos(CosTheta) * (180.0f / PI);
}

FVector UGPCameraMode_ThirdPerson::CalculateMidpoint(FVector const& LocationA, FVector const& LocationB)
{
	FVector MidPoint = (LocationA + LocationB) / 2;
	return MidPoint;
}

FRotator UGPCameraMode_ThirdPerson::CalculateRotationToMidpoint(FVector const& MidPoint, FVector const& ViewLocation)
{
	// Calculate the rotation vector and set the view rotation accordingly
	FVector RotationVector = MidPoint - ViewLocation;
	RotationVector.Normalize();
	FRotator ViewRotation = RotationVector.Rotation();
	return ViewRotation;
}

void UGPCameraMode_ThirdPerson::AdjustCameraIfNecessary(FVector const& LocationA, FVector const& LocationB, FVector ViewLocation, float DeltaTime)
{
	if (!bUseAutoFocus) return;

	FVector MidPoint = CalculateMidpoint(LocationA, LocationB);

	// Set rotation of the camera to look at midpoint
	FRotator ViewRotation = CalculateRotationToMidpoint(MidPoint, ViewLocation);

	float MinAngle = 30;
	CurrentViewLocation = View.Location;
	CurrentAngle = CalculateAngleBetweenVectors(LocationA - CurrentViewLocation, LocationB - CurrentViewLocation);

	if (CurrentAngle < MinAngle)
	{
		//FRotator NewRotation(View.Rotation.Pitch, View.Rotation.Yaw * (ViewRotation.Yaw - (MinAngle - CurrentAngle)), ViewRotation.Roll);
		FRotator NewRotation(ViewRotation.Pitch, ViewRotation.Yaw - (MinAngle - CurrentAngle), ViewRotation.Roll);
		ViewRotation = NewRotation;
		//ViewRotation = FMath::RInterpTo(ViewRotation, NewRotation, DeltaTime, InterpolationSpeed);
	}

#if ENABLE_DRAW_DEBUG


	UWorld* World = GetWorld();
	//FVector ForwardVector = (CurrentViewLocation + FVector::ForwardVector) - ViewLocation;
	DrawDebugSphere(World, MidPoint, 10, 8, FColor::Blue);
	DrawDebugLine(World, LocationA, LocationB, FColor::Blue);
	float Distance = FVector::Distance(LocationA, LocationB);
	DrawDebugString(World, LocationB + FVector(0.0f, 10.0f, 0.0f), FString::Printf(TEXT("Nearest Enemy Distance: (%f)"), Distance),(AActor*)0, Distance < 10 ? FColor::Red : FColor::White, DeltaTime, false, 1.0f);
	//DrawDebugSphere(World, CurrentViewLocation, 10, 8, FColor::Blue);
	//DrawDebugLine(World, CurrentViewLocation, View.Rotation.RotateVector(ForwardVector), FColor::Blue);

#endif

	View.Rotation = ViewRotation;

	// Calculate the view direction and new camera position
	FVector ViewForwardVector = ViewRotation.Vector();

	// Calculate the min distance from midpoint to have both actors in FOV
	float MinDistanceFromMidpoint = CalculateMinDistanceFromMidpoint(LocationA, LocationB, MidPoint, View.FieldOfView);

	float CurrentDist = FVector::Dist(ViewLocation, MidPoint);

	float DesiredDistanceFromMidpoint = FMath::Max(MinDistanceFromMidpoint, CurrentDist);

	// Adjust camera position by moving back
	FVector NewViewLocation = MidPoint - (ViewForwardVector * DesiredDistanceFromMidpoint);
	AdjustedAngle = CalculateAngleBetweenVectors(LocationA - NewViewLocation, LocationB - NewViewLocation);
	View.Location = NewViewLocation;


	// TODO
	// Smooth out the camera transition and rotation
	//View.Rotation = FMath::RInterpTo(View.Rotation, ViewRotation, DeltaTime, InterpolationSpeed);
	//View.Location = FMath::VInterpTo(View.Location, NewViewLocation, DeltaTime, InterpolationSpeed);
}

bool UGPCameraMode_ThirdPerson::IsActorInFOV(FVector ActorLocation)
{
	FVector Direction = ActorLocation - View.Location;
	Direction.Normalize();

	FVector ForwardVector = View.Rotation.Vector();

	float DotProduct = FVector::DotProduct(ForwardVector, Direction);
	float FOV = View.FieldOfView / 2;

	float AngleFromView = FMath::Acos(DotProduct) * (180.0f / PI);
	return AngleFromView <= (FOV / 2.0f);
}

float UGPCameraMode_ThirdPerson::CalculateMinDistanceFromMidpoint(const FVector& PlayerLocation, const FVector& EnemyLocation, const FVector& MidPoint, float FOVAngle)
{
	// Clamp the pitch to the range [ViewPitchMin, ViewPitchMax]
	float Pitch = FMath::Clamp(View.ControlRotation.Pitch, ViewPitchMin, ViewPitchMax);

	// Define the angle at pitch 0 and at pitch 90 or -90
	float AngleAtZeroPitch = FOVAngle;
	float AngleAtExtremePitch = FOVAngle / 4;

	// Calculate the interpolation parameter t based on the absolute value of the pitch
	float t = FMath::Abs(Pitch) / 90.0f;

	// Use a power function to curve the interpolation
	// Result will change faster as it approaches 0.
	float curvedT = FMath::Pow(t, 0.1f); // Adjust the exponent to control the curve

	// Interpolate the angle
	float OffsetAngle = FMath::Lerp(AngleAtZeroPitch, AngleAtExtremePitch, curvedT);

	// Calculate the distance between the player and enemy
	float DistanceBetweenActors = FVector::Dist(PlayerLocation, EnemyLocation);

	// Calculate the minimum distance to move back based on the OffsetAngle
	float DistanceToMoveBack = (DistanceBetweenActors / 2.0f) / FMath::Tan(FMath::DegreesToRadians(OffsetAngle));

	return DistanceToMoveBack;
}

void UGPCameraMode_ThirdPerson::PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly)
{
#if ENABLE_DRAW_DEBUG
	DebugActorsHitDuringCameraPenetration.Reset();
#endif

	float HardBlockedPct = DistBlockedPct;
	float SoftBlockedPct = DistBlockedPct;

	FVector BaseRay = CameraLoc - SafeLoc;
	FRotationMatrix BaseRayMatrix(BaseRay.Rotation());
	FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;

	BaseRayMatrix.GetScaledAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);

	float DistBlockedPctThisFrame = 1.f;

	int32 const NumRaysToShoot = bSingleRayOnly ? FMath::Min(1, PenetrationAvoidanceFeelers.Num()) : PenetrationAvoidanceFeelers.Num();
	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(CameraPen), false, nullptr/*PlayerCamera*/);

	SphereParams.AddIgnoredActor(&ViewTarget);

	//TODO ISegaiCameraTarget.GetIgnoredActorsForCameraPentration();
	//if (IgnoreActorForCameraPenetration)
	//{
	//	SphereParams.AddIgnoredActor(IgnoreActorForCameraPenetration);
	//}

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(0.f);
	UWorld* World = GetWorld();

	for (int32 RayIdx = 0; RayIdx < NumRaysToShoot; ++RayIdx)
	{
		FSegaiPenetrationAvoidanceFeeler& Feeler = PenetrationAvoidanceFeelers[RayIdx];
		if (Feeler.FramesUntilNextTrace <= 0)
		{
			// calc ray target
			FVector RayTarget;
			{
				FVector RotatedRay = BaseRay.RotateAngleAxis(Feeler.AdjustmentRot.Yaw, BaseRayLocalUp);
				RotatedRay = RotatedRay.RotateAngleAxis(Feeler.AdjustmentRot.Pitch, BaseRayLocalRight);
				RayTarget = SafeLoc + RotatedRay;
			}

			// cast for world and pawn hits separately.  this is so we can safely ignore the 
			// camera's target pawn
			SphereShape.Sphere.Radius = Feeler.Extent;
			ECollisionChannel TraceChannel = ECC_Camera;		//(Feeler.PawnWeight > 0.f) ? ECC_Pawn : ECC_Camera;

			// do multi-line check to make sure the hits we throw out aren't
			// masking real hits behind (these are important rays).

			// MT-> passing camera as actor so that camerablockingvolumes know when it's the camera doing traces
			FHitResult Hit;
			const bool bHit = World->SweepSingleByChannel(Hit, SafeLoc, RayTarget, FQuat::Identity, TraceChannel, SphereShape, SphereParams);
#if ENABLE_DRAW_DEBUG
			if (World->TimeSince(LastDrawDebugTime) < 1.f)
			{
				DrawDebugSphere(World, SafeLoc, SphereShape.Sphere.Radius, 8, FColor::Red);
				DrawDebugSphere(World, bHit ? Hit.Location : RayTarget, SphereShape.Sphere.Radius, 8, FColor::Red);
				DrawDebugLine(World, SafeLoc, bHit ? Hit.Location : RayTarget, FColor::Red);
			}
#endif // ENABLE_DRAW_DEBUG

			Feeler.FramesUntilNextTrace = Feeler.TraceInterval;

			const AActor* HitActor = Hit.GetActor();

			if (bHit && HitActor)
			{
				bool bIgnoreHit = false;

				if (Cast<APawn>(HitActor))
				//if (HitActor->ActorHasTag(GPCameraMode_ThirdPerson_Statics::NAME_IgnoreCameraCollision))
				{
					bIgnoreHit = true;
					SphereParams.AddIgnoredActor(HitActor);
				}

				// Ignore CameraBlockingVolume hits that occur in front of the ViewTarget.
				if (!bIgnoreHit && HitActor->IsA<ACameraBlockingVolume>())
				{
					const FVector ViewTargetForwardXY = ViewTarget.GetActorForwardVector().GetSafeNormal2D();
					const FVector ViewTargetLocation = ViewTarget.GetActorLocation();
					const FVector HitOffset = Hit.Location - ViewTargetLocation;
					const FVector HitDirectionXY = HitOffset.GetSafeNormal2D();
					const float DotHitDirection = FVector::DotProduct(ViewTargetForwardXY, HitDirectionXY);
					if (DotHitDirection > 0.0f)
					{
						bIgnoreHit = true;
						// Ignore this CameraBlockingVolume on the remaining sweeps.
						SphereParams.AddIgnoredActor(HitActor);
					}
					else
					{
#if ENABLE_DRAW_DEBUG
						DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif
					}
				}

				if (!bIgnoreHit)
				{
					float const Weight = Cast<APawn>(Hit.GetActor()) ? Feeler.PawnWeight : Feeler.WorldWeight;
					float NewBlockPct = Hit.Time;
					NewBlockPct += (1.f - NewBlockPct) * (1.f - Weight);

					// Recompute blocked pct taking into account pushout distance.
					NewBlockPct = ((Hit.Location - SafeLoc).Size() - CollisionPushOutDistance) / (RayTarget - SafeLoc).Size();
					DistBlockedPctThisFrame = FMath::Min(NewBlockPct, DistBlockedPctThisFrame);

					// This feeler got a hit, so do another trace next frame
					Feeler.FramesUntilNextTrace = 0;

#if ENABLE_DRAW_DEBUG
					DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif
				}
			}

			if (RayIdx == 0)
			{
				// don't interpolate toward this one, snap to it
				// assumes ray 0 is the center/main ray 
				HardBlockedPct = DistBlockedPctThisFrame;
			}
			else
			{
				SoftBlockedPct = DistBlockedPctThisFrame;
			}
		}
		else
		{
			--Feeler.FramesUntilNextTrace;
		}
	}

	if (bResetInterpolation)
	{
		DistBlockedPct = DistBlockedPctThisFrame;
	}
	else if (DistBlockedPct < DistBlockedPctThisFrame)
	{
		// interpolate smoothly out
		if (PenetrationBlendOutTime > DeltaTime)
		{
			DistBlockedPct = DistBlockedPct + DeltaTime / PenetrationBlendOutTime * (DistBlockedPctThisFrame - DistBlockedPct);
		}
		else
		{
			DistBlockedPct = DistBlockedPctThisFrame;
		}
	}
	else
	{
		if (DistBlockedPct > HardBlockedPct)
		{
			DistBlockedPct = HardBlockedPct;
		}
		else if (DistBlockedPct > SoftBlockedPct)
		{
			// interpolate smoothly in
			if (PenetrationBlendInTime > DeltaTime)
			{
				DistBlockedPct = DistBlockedPct - DeltaTime / PenetrationBlendInTime * (DistBlockedPct - SoftBlockedPct);
			}
			else
			{
				DistBlockedPct = SoftBlockedPct;
			}
		}
	}

	DistBlockedPct = FMath::Clamp<float>(DistBlockedPct, 0.f, 1.f);
	if (DistBlockedPct < (1.f - ZERO_ANIMWEIGHT_THRESH))
	{
		CameraLoc = SafeLoc + (CameraLoc - SafeLoc) * DistBlockedPct;
	}
}

void UGPCameraMode_ThirdPerson::DrawDebug(UCanvas* Canvas) const
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

	DisplayDebugManager.DrawString(
		FString::Printf(TEXT("Current Target Actors Angle: %f")
			, CurrentAngle));

	DisplayDebugManager.DrawString(
		FString::Printf(TEXT("Adjusted Target Actors Angle: %f")
			, AdjustedAngle));

	LastDrawDebugTime = GetWorld()->GetTimeSeconds();
#endif
}

void UGPCameraMode_ThirdPerson::SetTargetCrouchOffset(FVector NewTargetOffset)
{
	CrouchOffsetBlendPct = 0.0f;
	InitialCrouchOffset = CurrentCrouchOffset;
	TargetCrouchOffset = NewTargetOffset;
}

void UGPCameraMode_ThirdPerson::UpdateCrouchOffset(float DeltaTime)
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

void UGPCameraMode_ThirdPerson::SetTargetViewLocation(FVector NewTargetOffset)
{
	ViewLocationBlendPct = 0.0f;
	InitialViewLocation = CurrentViewLocation;
	TargetViewLocation = NewTargetOffset;
}

void UGPCameraMode_ThirdPerson::UpdateViewLocation(float DeltaTime)
{
	if (ViewLocationBlendPct < 1.0f)
	{
		ViewLocationBlendPct = FMath::Min(ViewLocationBlendPct + DeltaTime * ViewLocationBlendMultiplier, 1.0f);
		CurrentViewLocation = FMath::InterpEaseInOut(InitialViewLocation, TargetViewLocation, ViewLocationBlendPct, 1.0f);
	}
	else
	{
		CurrentViewLocation = TargetViewLocation;
		ViewLocationBlendPct = 1.0f;
	}
}
