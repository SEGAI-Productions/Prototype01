// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EventBindingsSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FDamageResult
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	AActor* Source;

	UPROPERTY(BlueprintReadWrite)
	AActor* Target;

	UPROPERTY(BlueprintReadWrite)
	float Damage;
};

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UEventBindingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamageResultDynamicDelegate, const FDamageResult&, DamageResult);

public:
	UFUNCTION(BlueprintCallable)
	void HandleDamageInstigated(FDamageResult DamageResult);

	UPROPERTY(BlueprintAssignable)
	FDamageResultDynamicDelegate OnDamageInstigatedDynamic;
	
};
