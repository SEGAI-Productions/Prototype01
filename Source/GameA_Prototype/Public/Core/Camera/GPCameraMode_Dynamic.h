// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core/Camera/GPCameraMode_ThirdPerson.h"
#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "GPCameraMode_Dynamic.generated.h"

class UCurveVector;

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class GAMEA_PROTOTYPE_API UGPCameraMode_Dynamic : public UGPCameraMode_ThirdPerson
{
	GENERATED_BODY()

public:

	UGPCameraMode_Dynamic();
	void SetDynamicOffsetCurve(TObjectPtr<const UCurveVector> DynamicOffset);

protected:

	virtual void UpdateView(float DeltaTime) override;

	// Called when this camera mode is activated on the camera mode stack.
	virtual void OnActivation();

	// Called when this camera mode is deactivated on the camera mode stack.
	virtual void OnDeactivation();

protected:

	float ElapsedTime = 0.0f;

	// Curve that defines local-space offsets from the target using the view pitch to evaluate the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Dynamic", Meta = (EditCondition = "!bUseRuntimeFloatCurves"))
	TObjectPtr<const UCurveVector> DynamicOffsetCurve;
};
