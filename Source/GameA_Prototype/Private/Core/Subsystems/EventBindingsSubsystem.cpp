// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystems/EventBindingsSubsystem.h"

void UEventBindingsSubsystem::HandleDamageInstigated(FDamageResult DamageResult)
{
	OnDamageInstigatedDynamic.Broadcast(DamageResult);
}