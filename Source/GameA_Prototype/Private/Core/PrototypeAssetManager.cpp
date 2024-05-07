// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PrototypeAssetManager.h"
#include "Core/Tags/PrototypeGameplayTags.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PrototypeAssetManager)
 
UPrototypeAssetManager::UPrototypeAssetManager() {}

void UPrototypeAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	UAbilitySystemGlobals::Get().InitGlobalData();

	//Load Native Tags
	FPrototypeGameplayTags::InitializeNativeTags();

	UE_LOG(LogTemp, Warning, TEXT("AsssetManager Initialized!"));
}

UPrototypeAssetManager& UPrototypeAssetManager::Get()
{
	check(GEngine);

	return *Cast<UPrototypeAssetManager>(GEngine->AssetManager);
	UE_LOG(LogTemp, Warning, TEXT("AssetManager Initialized!"));

	//if (UPrototypeAssetManager* Singleton = Cast<UPrototypeAssetManager>(GEngine->AssetManager))
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("AssetManager Initialized!"));
	//	return *Singleton;
	//}

	//UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to GPAssetManager!"));

	//// Fatal error above prevents this from being called.
	//return *NewObject<UGPAssetManager>();
}