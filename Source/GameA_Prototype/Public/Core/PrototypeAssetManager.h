// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "PrototypeAssetManager.generated.h"

/**
 * Used to initialize gameplay tags on engine startup.
 */
UCLASS()
class GAMEA_PROTOTYPE_API UPrototypeAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UPrototypeAssetManager();
	static UPrototypeAssetManager& Get(); // Returns the AssetManager singleton object.

protected:
	virtual void StartInitialLoading() override;
};
