// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GPCharacterMovementComponent.generated.h"

class UObject;
struct FFrame;

/**
 * 
 */
USTRUCT(BlueprintType)
struct FGPCharacterGroundInfo
{
	GENERATED_BODY()

	FGPCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

UCLASS(Config = Game)
class GAMEA_PROTOTYPE_API UGPCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UGPCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void SimulateMovement(float DeltaTime) override;

	virtual bool CanAttemptJump() const override;

	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "GP|CharacterMovement")
	const FGPCharacterGroundInfo& GetGroundInfo();

	void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent interface

protected:

	virtual void InitializeComponent() override;

protected:

	// Cached ground info for the character.  Do not access this directly!  It's only updated when accessed via GetGroundInfo().
	FGPCharacterGroundInfo CachedGroundInfo;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;
	
};
