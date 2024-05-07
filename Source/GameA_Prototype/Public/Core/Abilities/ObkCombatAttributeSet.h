#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ObkCombatAttributeSet.generated.h"

UCLASS()
class GAMEA_PROTOTYPE_API UObkCombatAttributeSet : public UAttributeSet {
	GENERATED_BODY()

public:
	UObkCombatAttributeSet();
	void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	FGameplayAttributeData Health;
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Health);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(Health);
	//float GetHealth() const;
	//void SetHealth(float NewVal);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(Health);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UObkCombatAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	FGameplayAttributeData MaxHealth;
	GAMEPLAYATTRIBUTE_VALUE_GETTER(MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxHealth);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UObkCombatAttributeSet, MaxHealth);
};
