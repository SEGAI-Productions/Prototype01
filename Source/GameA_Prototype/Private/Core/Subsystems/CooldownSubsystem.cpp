// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystems/CooldownSubsystem.h"

DEFINE_LOG_CATEGORY(LogCooldown);

#define CD_LOG_DISP_TIME 5.f

void UCooldownSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
  UE_LOG(LogCooldown, Log, TEXT("CooldownSubsystem Initialized"), *GetName());
}

void UCooldownSubsystem::Deinitialize()
{

}

void UCooldownSubsystem::ApplyCooldown(TSubclassOf<class UObject> CooldownClass, AActor* CooldownOwner, float CooldownLength)
{
  if (bPruneOnEachApply)
    PruneOutdatedCooldowns();

  if (CooldownOwner == nullptr)
    return;

  UGameInstance* GameInstance = GetGameInstance();
  if (GameInstance == nullptr)
    return;

  if (GEngine == nullptr)
    return;

  UWorld* World = GEngine->GetWorldFromContextObject(GameInstance, EGetWorldErrorMode::LogAndReturnNull);
  if (World == nullptr)
    return;

  float CurrentTime = World->GetTimeSeconds();
  bool bFound = false;
  TWeakObjectPtr<AActor> OwnerWeakPtr = CooldownOwner;
  FCooldownEntryContainer& Container = PerActorCooldowns.FindOrAdd(OwnerWeakPtr);
  FCooldownEntry& Entry = Container.CooldownList.FindOrAdd(CooldownClass);
  Entry.CooldownOwner = CooldownOwner;
  Entry.CooldownClass = CooldownClass;
  Entry.CooldownDuration = CooldownLength;
  Entry.GameTimeSecondsWhenApplied = CurrentTime;
  Entry.GameTimeSecondsWhenComplete = CurrentTime + CooldownLength;
      
  FString DisplayStr = FString::Printf(TEXT("Cooldown Applied: %s to %s for %f"), *CooldownClass->GetName(), *CooldownOwner->GetName(), CooldownLength);
  UE_LOG(LogCooldown, Log, TEXT("%s"), *DisplayStr, *GetName());
  //if (GEngine)
  //{
  //  GEngine->AddOnScreenDebugMessage(INDEX_NONE, CD_LOG_DISP_TIME, FColor::Orange, DisplayStr);
  //}
}

void UCooldownSubsystem::ApplyGlobalCooldown(TSubclassOf<class UObject> CooldownClass, AActor* CooldownOwner, float CooldownLength)
{
  if (bPruneOnEachApply)
    PruneOutdatedCooldowns();

  UGameInstance* GameInstance = GetGameInstance();
  if (GameInstance == nullptr)
    return;

  if (GEngine == nullptr)
    return;

  UWorld* World = GEngine->GetWorldFromContextObject(GameInstance, EGetWorldErrorMode::LogAndReturnNull);
  if (World == nullptr)
    return;

  float CurrentTime = World->GetTimeSeconds();

  FCooldownEntry& Entry = GlobalCooldowns.FindOrAdd(CooldownClass);
  Entry.CooldownClass = CooldownClass;
  Entry.CooldownOwner = CooldownOwner;
  Entry.CooldownDuration = CooldownLength;
  Entry.GameTimeSecondsWhenApplied = CurrentTime;
  Entry.GameTimeSecondsWhenComplete = CurrentTime + CooldownLength;

  FString DisplayStr = FString::Printf(TEXT("Global Cooldown Applied: %s by %s for %f"), *CooldownClass->GetName(), *CooldownOwner->GetName(), CooldownLength);
  UE_LOG(LogCooldown, Log, TEXT("%s"), *DisplayStr, *GetName());
  //if (GEngine)
  //{    
  //  GEngine->AddOnScreenDebugMessage(INDEX_NONE, CD_LOG_DISP_TIME, FColor::Orange, DisplayStr);
  //}
}

bool UCooldownSubsystem::IsOnCooldown(TSubclassOf<class UObject> CooldownClass, AActor* CooldownOwner)
{
  float TimeRemaining = 0;
  float CooldownDuration = 0;
  GetCooldownTimeRemainingAndDuration(CooldownClass, CooldownOwner, TimeRemaining, CooldownDuration);
  return TimeRemaining > 0;
}

void UCooldownSubsystem::GetCooldownTimeRemainingAndDuration(TSubclassOf<class UObject> CooldownClass, AActor* CooldownOwner, float& TimeRemaining, float& CooldownDuration)
{
  TimeRemaining = 0;
  CooldownDuration = 0;

  UGameInstance* GameInstance = GetGameInstance();
  if (GameInstance == nullptr)
    return;

  if (GEngine == nullptr)
    return;

  UWorld* World = GEngine->GetWorldFromContextObject(GameInstance, EGetWorldErrorMode::LogAndReturnNull);
  if (World == nullptr)
    return;

  float CurrentTime = World->GetTimeSeconds();

  // Check global cooldowns
  {
    FCooldownEntry* Entry = GlobalCooldowns.Find(CooldownClass);
    if (Entry != nullptr)
    {
      CooldownDuration = Entry->CooldownDuration;
      TimeRemaining = FMath::Max(0, Entry->GameTimeSecondsWhenComplete - CurrentTime);
      if (TimeRemaining <= 0)
        GlobalCooldowns.Remove(CooldownClass);
    }
  }

  // Check per-actor cooldowns
  TWeakObjectPtr<AActor> OwnerWeakPtr = CooldownOwner;
  FCooldownEntryContainer* Container = PerActorCooldowns.Find(OwnerWeakPtr);
  if (Container != nullptr)
  {
    FCooldownEntry* Entry = Container->CooldownList.Find(CooldownClass);
    if (Entry != nullptr)
    {
      CooldownDuration = FMath::Max(CooldownDuration, Entry->CooldownDuration);
      TimeRemaining = FMath::Max(0, Entry->GameTimeSecondsWhenComplete - CurrentTime);
      if (TimeRemaining <= 0)
        Container->CooldownList.Remove(CooldownClass);
    }
  }
}

void UCooldownSubsystem::PruneOutdatedCooldowns()
{
  UGameInstance* GameInstance = GetGameInstance();
  if (GameInstance == nullptr)
    return;

  if (GEngine == nullptr)
    return;

  UWorld* World = GEngine->GetWorldFromContextObject(GameInstance, EGetWorldErrorMode::LogAndReturnNull);
  if (World == nullptr)
    return;

  float CurrentTime = World->GetTimeSeconds();

  // Prune global cooldowns
  {
    GlobalCooldowns.GetKeys(GlobalKeys);

    for (TSubclassOf<UObject> ObjClass : GlobalKeys)
    {
      if (FCooldownEntry* Entry = GlobalCooldowns.Find(ObjClass))
      {
        if (Entry->GameTimeSecondsWhenComplete - CurrentTime <= 0)
          GlobalCooldowns.Remove(ObjClass);
      }
      else
      {
        GlobalCooldowns.Remove(ObjClass);
      }
    }
  }

  // Prune Actor Cooldowns
  {
    PerActorCooldowns.GetKeys(ActorKeys);

    for (TWeakObjectPtr ActorWeakPtr : ActorKeys)
    {
      AActor* ActorPtr = ActorWeakPtr.Get();
      if (ActorPtr == nullptr)
      {
        PerActorCooldowns.Remove(ActorWeakPtr);
        continue;
      }

      FCooldownEntryContainer* Container = PerActorCooldowns.Find(ActorWeakPtr);
      if (Container == nullptr)
      {
        PerActorCooldowns.Remove(ActorWeakPtr);
        continue;
      }

      Container->CooldownList.GetKeys(ActorCooldownKeys);

      for (TSubclassOf<UObject> ObjClass : ActorCooldownKeys)
      {
        if (FCooldownEntry* Entry = Container->CooldownList.Find(ObjClass))
        {
          if (Entry->GameTimeSecondsWhenComplete - CurrentTime <= 0)
          {
            Container->CooldownList.Remove(ObjClass);
          }
        }
        else
        {
          Container->CooldownList.Remove(ObjClass);
        }        
      }

      if (Container->CooldownList.Num() == 0)
      {
        PerActorCooldowns.Remove(ActorWeakPtr);
      }
    }
  }
}