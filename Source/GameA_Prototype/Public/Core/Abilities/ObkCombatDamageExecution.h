#pragma once
#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ObkCombatDamageExecution.generated.h"

UCLASS()
class GAMEA_PROTOTYPE_API UObkCombatDamageExecution : public UGameplayEffectExecutionCalculation {
	GENERATED_BODY()
public:
	UObkCombatDamageExecution();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
