// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Camera/SegaiCameraAssistInterface.h"
#include "GameFramework/PlayerController.h"
#include "SegaiPlayerController.generated.h"

class FPrimitiveComponentId;
struct FFrame;

/**
 * 
 */
UCLASS(Config = Game)
class GAMEA_PROTOTYPE_API ASegaiPlayerController : public APlayerController, public ISegaiCameraAssistInterface
{
	GENERATED_BODY()

public:
	ASegaiPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#pragma region APlayerController interface
	virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
#pragma endregion
	
#pragma region Camera Assist Interface
	virtual void OnCameraPenetratingTarget() override;
#pragma endregion

protected:

	bool bHideViewTargetPawnNextFrame = false;
};
