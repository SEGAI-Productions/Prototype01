// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystems/CooldownSubsystem.h"

DEFINE_LOG_CATEGORY(LogCooldown);

void UCooldownSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
  UE_LOG(LogCooldown, Log, TEXT("CooldownSubsystem Initialized"), *GetName());
}

void UCooldownSubsystem::Deinitialize()
{

}

void UCooldownSubsystem::ApplyCooldown(TSubclassOf<class UObject> CooldownClass, AActor* CooldownOwner, float CooldownLength)
{
  if (CooldownOwner == nullptr)
    return;

  UGameInstance* GameInstance = GetGameInstance();
  if (GameInstance == nullptr)
    return;

  UWorld* World = GEngine->GetWorldFromContextObject(GameInstance, EGetWorldErrorMode::LogAndReturnNull);
  if (World == nullptr)
    return;

  float CurrentTime = World->GetTimeSeconds();
  bool bFound = false;
  TWeakObjectPtr<AActor> OwnerWeakPtr = CooldownOwner;
  FCooldownEntryContainer Container = PerActorCooldowns.FindOrAdd(OwnerWeakPtr);
  for (FCooldownEntry Entry : Container.CooldownList)
  {
    if (Entry.CooldownClass == CooldownClass)
    {
      Entry.CooldownDuration = CooldownLength;
      Entry.GameTimeSecondsWhenApplied = CurrentTime;
      Entry.GameTimeSecondsWhenComplete = CurrentTime + CooldownLength;
      bFound = true;
      break;
    }
  }

  if (!bFound)
  {
    FCooldownEntry NewEntry;
    NewEntry.CooldownOwner = CooldownOwner;
    NewEntry.CooldownClass = CooldownClass;
    NewEntry.CooldownDuration = CooldownLength;
    NewEntry.GameTimeSecondsWhenApplied = CurrentTime;
    NewEntry.GameTimeSecondsWhenComplete = CurrentTime + CooldownLength;

    Container.CooldownList.Add(NewEntry);
  }
}

void UCooldownSubsystem::ApplyGlobalCooldown(TSubclassOf<class UObject> CooldownClass, AActor* CooldownOwner, float CooldownLength)
{
  UGameInstance* GameInstance = GetGameInstance();
  if (GameInstance == nullptr)
    return;

  UWorld* World = GEngine->GetWorldFromContextObject(GameInstance, EGetWorldErrorMode::LogAndReturnNull);
  if (World == nullptr)
    return;

  float CurrentTime = World->GetTimeSeconds();

  FCooldownEntry Entry = GlobalCooldowns.FindOrAdd(CooldownClass);
  Entry.CooldownOwner = CooldownOwner;
  Entry.CooldownDuration = CooldownLength;
  Entry.GameTimeSecondsWhenApplied = CurrentTime;
  Entry.GameTimeSecondsWhenComplete = CurrentTime + CooldownLength;
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
    bool bClearCooldown = false;
    int CurrentIndex = INDEX_NONE;
    for (const FCooldownEntry Entry : Container->CooldownList)
    {
      CurrentIndex++;
      if (Entry.CooldownClass == CooldownClass)
      {
        CooldownDuration = FMath::Max(CooldownDuration, Entry.CooldownDuration);
        float InstanceTimeRemaining = FMath::Max(0, Entry.GameTimeSecondsWhenComplete - CurrentTime);
        TimeRemaining = FMath::Max(TimeRemaining, InstanceTimeRemaining);
        bClearCooldown = InstanceTimeRemaining <= 0;
        break;
      }
    }
    if (bClearCooldown)
      Container->CooldownList.RemoveAt(CurrentIndex);
  }
}