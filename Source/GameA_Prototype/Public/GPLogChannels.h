// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

GAMEA_PROTOTYPE_API DECLARE_LOG_CATEGORY_EXTERN(LogGP, Log, All);
GAMEA_PROTOTYPE_API DECLARE_LOG_CATEGORY_EXTERN(LogGPExperience, Log, All);
GAMEA_PROTOTYPE_API DECLARE_LOG_CATEGORY_EXTERN(LogGPAbilitySystem, Log, All);
GAMEA_PROTOTYPE_API DECLARE_LOG_CATEGORY_EXTERN(LogGPTeams, Log, All);

GAMEA_PROTOTYPE_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
