// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EventBindingsSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FEBSDamageResult
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	AActor* Source;

	UPROPERTY(BlueprintReadWrite)
	AActor* Target;

	UPROPERTY(BlueprintReadWrite)
	float DamageDealt;

	UPROPERTY(BlueprintReadWrite)
	float DamageAssigned;
};

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UEventBindingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEBSDamageResultDynamicDelegate, const FEBSDamageResult&, DamageResult);

public:
	UFUNCTION(BlueprintCallable)
	void HandleEBSDamageResult(FEBSDamageResult DamageResult);

	UPROPERTY(BlueprintAssignable)
	FEBSDamageResultDynamicDelegate OnEBSDamageResultDynamic;
	
};
