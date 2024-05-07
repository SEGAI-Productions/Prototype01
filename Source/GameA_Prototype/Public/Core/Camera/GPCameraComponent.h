// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"

#include "GPCameraComponent.generated.h"

class UCanvas;
class UGPCameraMode;
class UGPCameraModeStack;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FMinimalViewInfo;
template <class TClass> class TSubclassOf;

DECLARE_DELEGATE_RetVal(TSubclassOf<UGPCameraMode>, FGPCameraModeDelegate);

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UGPCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:

	UGPCameraComponent(const FObjectInitializer& ObjectInitializer);

	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintPure)
	static UGPCameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UGPCameraComponent>() : nullptr); }

	UFUNCTION(BlueprintCallable)
	void SetCameraMode(TSubclassOf<UGPCameraMode> CameraMode);

	// Returns the target actor that the camera is looking at.
	virtual AActor* GetTargetActor() const { return GetOwner(); }

	// Delegate used to query for the best camera mode.
	FGPCameraModeDelegate DetermineCameraModeDelegate;

	// Add an offset to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	void AddFieldOfViewOffset(float FovOffset) { FieldOfViewOffset += FovOffset; }

	virtual void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:

	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	virtual void UpdateCameraModes();

public:
	// Stack used to blend the camera modes.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, category = "Camera")
	TSubclassOf<UGPCameraMode> DefaultCameraMode;

protected:


	UPROPERTY()
	TObjectPtr<UGPCameraModeStack> CameraModeStack;

	// Offset applied to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	float FieldOfViewOffset;
};
