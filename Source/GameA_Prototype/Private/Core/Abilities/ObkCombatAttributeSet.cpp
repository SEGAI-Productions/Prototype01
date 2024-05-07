#include "Core/Abilities/ObkCombatAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

UObkCombatAttributeSet::UObkCombatAttributeSet() {};

void UObkCombatAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) {
	Super::PostGameplayEffectExecute(Data);
	//if (Data.EvaluatedData.Attribute == GetHealthAttribute()) {
	//	SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	//}
}

//float UObkCombatAttributeSet::GetHealth() const {
//	float current = Health.GetCurrentValue();
//	return FMath::Max(current, 0.0f);
//}
//
//void UObkCombatAttributeSet::SetHealth(float NewVal) {
//    NewVal = FMath::Max(NewVal, 0.0f);
//    UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
//    if (ensure(ASC)) { ASC->SetNumericAttributeBase(GetHealthAttribute(), NewVal); }
//}