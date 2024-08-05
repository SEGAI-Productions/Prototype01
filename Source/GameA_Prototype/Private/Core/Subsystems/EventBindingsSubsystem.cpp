// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystems/EventBindingsSubsystem.h"

void UEventBindingsSubsystem::HandleEBSDamageResult(FEBSDamageResult DamageResult)
{
	OnEBSDamageResultDynamic.Broadcast(DamageResult);
}