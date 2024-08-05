// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "GPCameraMode.generated.h"

class AActor;
class UCanvas;
class UCurveVector;
class UGPCameraComponent;
class USkeletalMeshSocket;

#define GP_CAMERA_DEFAULT_FOV		(80.0f)
#define GP_CAMERA_DEFAULT_PITCH_MIN	(-89.0f)
#define GP_CAMERA_DEFAULT_PITCH_MAX	(89.0f)

/**
 * EGPCameraModeBlendFunction
 *
 *	Blend function used for transitioning between camera modes.
 */
UENUM(BlueprintType)
enum class EGPCameraModeBlendFunction : uint8
{
	// Does a simple linear interpolation.
	Linear,

	// Immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by the exponent.
	EaseIn,

	// Smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by the exponent.
	EaseOut,

	// Smoothly accelerates and decelerates.  Ease amount controlled by the exponent.
	EaseInOut,

	COUNT	UMETA(Hidden)
};

/**
 * FGPCameraModeView
 *
 *	View data produced by the camera mode that is used to blend camera modes.
 */
struct FGPCameraModeView
{
public:

	FGPCameraModeView();

	void Blend(const FGPCameraModeView& Other, float OtherWeight);

public:

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};

/**
 * Base class for all camera modes.
 */
UCLASS(Abstract, NotBlueprintable)
class GAMEA_PROTOTYPE_API UGPCameraMode : public UObject
{
	GENERATED_BODY()

public:

	UGPCameraMode();

	UGPCameraComponent* GetGPCameraComponent() const;

	virtual UWorld* GetWorld() const override;

	AActor* GetTargetActor() const;

	const FGPCameraModeView& GetCameraModeView() const { return View; }

	// Called when this camera mode is activated on the camera mode stack.
	virtual void OnActivation() {};

	// Called when this camera mode is deactivated on the camera mode stack.
	virtual void OnDeactivation() {};

	void UpdateCameraMode(float DeltaTime);

	virtual void SetFocusActor(AActor* NewFocusActor);
	void FocusSocketByName(FName FocusSocket);
	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	void SetBlendWeight(float Weight);

	FGameplayTag GetCameraTypeTag() const
	{
		return CameraTypeTag;
	}

	virtual void DrawDebug(UCanvas* Canvas) const;

protected:

	virtual FVector GetPivotLocation() const;
	virtual FRotator GetPivotRotation() const;

	virtual void UpdateView(float DeltaTime);
	virtual void UpdateBlending(float DeltaTime);

protected:
	// A tag that can be queried by gameplay code that cares when a kind of camera mode is active
	// without having to ask about a specific mode (e.g., when aiming downsights to get more accuracy)
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	FGameplayTag CameraTypeTag;

	// View output produced by the camera mode.
	FGPCameraModeView View;

	// The horizontal field of view (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView;

	// Minimum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin;

	// Maximum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax;

	// How long it takes to blend in this mode.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	// Function used for blending.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	EGPCameraModeBlendFunction BlendFunction;

	// Exponent used by blend functions to control the shape of the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent;

	// Linear blend alpha used to determine the blend weight.
	float BlendAlpha;

	// Blend weight calculated using the blend alpha and function.
	float BlendWeight;

	AActor* FocusActor = nullptr;
	FName FocusSocketName;

protected:
	/** If true, skips all interpolation and puts camera in ideal location.  Automatically set to false next frame. */
	UPROPERTY(transient)
	uint32 bResetInterpolation : 1;

#pragma region CameraLag

	/**
	 * If true, camera lags behind target position to smooth its movement.
	 * @see CameraLagSpeed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag)
	uint32 bEnableCameraLag : 1;

	/**
	 * If true, camera lags behind target rotation to smooth its movement.
	 * @see CameraRotationLagSpeed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag)
	uint32 bEnableCameraRotationLag : 1;

	/**
	 * If bUseCameraLagSubstepping is true, sub-step camera damping so that it handles fluctuating frame rates well (though this comes at a cost).
	 * @see CameraLagMaxTimeStep
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, AdvancedDisplay)
	uint32 bUseCameraLagSubstepping : 1;

	/**
	 * If true and camera location lag is enabled, draws markers at the camera target (in green) and the lagged position (in yellow).
	 * A line is drawn between the two locations, in green normally but in red if the distance to the lag target has been clamped (by CameraLagMaxDistance).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag)
	uint32 bDrawDebugLagMarkers : 1;

	/** If bEnableCameraLag is true, controls how quickly camera reaches target position. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, meta = (editcondition = "bEnableCameraLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraLagSpeed;

	/** If bEnableCameraRotationLag is true, controls how quickly camera reaches target position. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, meta = (editcondition = "bEnableCameraRotationLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraRotationLagSpeed;

	/** Max time step used when sub-stepping camera lag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, AdvancedDisplay, meta = (editcondition = "bUseCameraLagSubstepping", ClampMin = "0.005", ClampMax = "0.5", UIMin = "0.005", UIMax = "0.5"))
	float CameraLagMaxTimeStep;

	/** Max distance the camera target may lag behind the current location. If set to zero, no max distance is enforced. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, meta = (editcondition = "bEnableCameraLag", ClampMin = "0.0", UIMin = "0.0"))
	float CameraLagMaxDistance;

	/** Temporary variables when using camera lag, to record previous camera position */
	FVector PreviousDesiredLoc;

	/** Temporary variable for lagging camera rotation, for previous rotation */
	FRotator PreviousDesiredRot;

	virtual void UpdateCameraLag(float DeltaTime, FVector& PivotLocation, FRotator& PivotRotation);

#pragma endregion

};


/**
 * UGPCameraModeStack
 *
 *	Stack used for blending camera modes.
 */
UCLASS()
class UGPCameraModeStack : public UObject
{
	GENERATED_BODY()

public:

	UGPCameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const { return bIsActive; }

	void PushCameraMode(TSubclassOf<UGPCameraMode> CameraModeClass);

	bool EvaluateStack(float DeltaTime, FGPCameraModeView& OutCameraModeView);

	void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;
	void SetFocusObject(TSubclassOf<UGPCameraMode> CameraModeClass, AActor* NewFocusObject);
	void SetDynamicOffsetCurve(TSubclassOf<UGPCameraMode> CameraMode, TObjectPtr<UCurveVector> DynamicOffset);
	void FocusSocketByName(TSubclassOf<UGPCameraMode> CameraMode, FName FocusSocket);

protected:

	UGPCameraMode* GetCameraModeInstance(TSubclassOf<UGPCameraMode> CameraModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FGPCameraModeView& OutCameraModeView) const;

protected:

	bool bIsActive;

	UPROPERTY()
	TArray<TObjectPtr<UGPCameraMode>> CameraModeInstances;

	UPROPERTY()
	TArray<TObjectPtr<UGPCameraMode>> CameraModeStack;
};
