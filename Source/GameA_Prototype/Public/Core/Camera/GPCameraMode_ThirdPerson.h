// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Camera/GPCameraMode.h"
#include "Curves/CurveFloat.h"
#include "Core/Camera/SegaiPenetrationAvoidanceFeeler.h"
#include "GPCameraMode_ThirdPerson.generated.h"

class UCurveVector;

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class GAMEA_PROTOTYPE_API UGPCameraMode_ThirdPerson : public UGPCameraMode
{
	GENERATED_BODY()

public:

	UGPCameraMode_ThirdPerson();

protected:

	virtual void UpdateView(float DeltaTime) override;
	FVector GetFocusLocation();
	void UpdateForTarget(float DeltaTime);
	void UpdatePreventPenetration(float DeltaTime); 
	float CalculateAngleBetweenVectors(const FVector& A, const FVector& B);
	FVector CalculateMidpoint(FVector const& LocationA, FVector const& LocationB);
	FRotator CalculateRotationToMidpoint(FVector const& MidPoint, FVector const& ViewLocation);
	void AdjustCameraIfNecessary(FVector const& LocationA, FVector const& LocationB, FVector ViewLocation, float DeltaTime);
	bool IsActorInFOV(FVector ActorLocation);
	float CalculateMinDistanceFromMidpoint(const FVector& PlayerLocation, const FVector& EnemyLocation, const FVector& MidPoint, float FOVAngle);
	void PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly);

	virtual void DrawDebug(UCanvas* Canvas) const override;

protected:

	// Curve that defines local-space offsets from the target using the view pitch to evaluate the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "!bUseRuntimeFloatCurves"))
	TObjectPtr<const UCurveVector> TargetOffsetCurve;

	// UE-103986: Live editing of RuntimeFloatCurves during PIE does not work (unlike curve assets).
	// Once that is resolved this will become the default and TargetOffsetCurve will be removed.
	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	bool bUseRuntimeFloatCurves;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	bool bUseAutoFocus = true;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetX;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetY;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetZ;

	// Alters the speed that a crouch offset is blended in or out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
	float CrouchOffsetBlendMultiplier = 5.0f;

	// Alters the speed that a crouch offset is blended in or out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
	float ViewLocationBlendMultiplier = 5.0f;

	// Penetration prevention
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float PenetrationBlendInTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float PenetrationBlendOutTime = 0.15f;

	/** If true, does collision checks to keep the camera out of the world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bPreventPenetration = true;

	/** If true, try to detect nearby walls and move the camera in anticipation.  Helps prevent popping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bDoPredictiveAvoidance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionPushOutDistance = 2.f;

	/** When the camera's distance is pushed into this percentage of its full distance due to penetration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float ReportPenetrationPercent = 0.f;

	// Alters the speed that a crouch offset is blended in or out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
	float InterpolationSpeed = .5f;

	/**
	 * These are the feeler rays that are used to find where to place the camera.
	 * Index: 0  : This is the normal feeler we use to prevent collisions.
	 * Index: 1+ : These feelers are used if you bDoPredictiveAvoidance=true, to scan for potential impacts if the player
	 *             were to rotate towards that direction and primitively collide the camera so that it pulls in before
	 *             impacting the occluder.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FSegaiPenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;

	UPROPERTY(Transient)
	float AimLineToDesiredPosBlockedPct;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const AActor>> DebugActorsHitDuringCameraPenetration;

#if ENABLE_DRAW_DEBUG
	mutable float LastDrawDebugTime = -MAX_FLT;
#endif

protected:

	float CurrentAngle = 0.0f;
	float AdjustedAngle = 0.0f;

	float CrouchOffsetBlendPct = 1.0f;
	FVector TargetOffset = FVector::ZeroVector;
	FVector InitialCrouchOffset = FVector::ZeroVector;
	FVector TargetCrouchOffset = FVector::ZeroVector;
	FVector CurrentCrouchOffset = FVector::ZeroVector;

	float ViewLocationBlendPct = 1.0f;
	FVector InitialViewLocation = FVector::ZeroVector;
	FVector TargetViewLocation = FVector::ZeroVector;
	FVector CurrentViewLocation = FVector::ZeroVector;

	void SetTargetCrouchOffset(FVector NewTargetOffset);
	void UpdateCrouchOffset(float DeltaTime);

	void SetTargetViewLocation(FVector NewTargetOffset);
	void UpdateViewLocation(float DeltaTime);
};
