#include "Core/Abilities/ObkCombatDamageExecution.h"
#include "Core/Abilities/ObkCombatAttributeSet.h"
#include "AbilitySystemComponent.h"

struct FCombatDamageStatics {
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);

	FCombatDamageStatics() {
		DEFINE_ATTRIBUTE_CAPTUREDEF(UObkCombatAttributeSet, Health, Target, false);
	}
};

static const FCombatDamageStatics& CombatDamageStatics() {
	static FCombatDamageStatics DmgStatics;
	return DmgStatics;
}

UObkCombatDamageExecution::UObkCombatDamageExecution() {}

void UObkCombatDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const {
	UAbilitySystemComponent* TargetABSC = ExecutionParams.GetTargetAbilitySystemComponent();
	AActor* TargetActor = TargetABSC ? TargetABSC->GetAvatarActor() : nullptr;

	UAbilitySystemComponent* SourceABSC = ExecutionParams.GetSourceAbilitySystemComponent();
	AActor* SourceActor = SourceABSC ? SourceABSC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float DamageDone = 10.0f;

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(CombatDamageStatics().HealthProperty, EGameplayModOp::Additive, -DamageDone));
}
