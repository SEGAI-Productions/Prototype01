#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Core/Abilities/ObkCombatAttributeSet.h"
#include "OberakCharacterBase.generated.h"

UCLASS()
class GAMEA_PROTOTYPE_API AOberakCharacterBase : public ACharacter, public IAbilitySystemInterface {
	GENERATED_BODY()

public:
	AOberakCharacterBase(const class FObjectInitializer& ObjectInitializer);
protected:
	virtual void BeginPlay() override;
	void InitializeAttributes();
public:	
	virtual void PossessedBy(AController* NewController) override;
	//Ability
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<class UOberakAbilitySystemComponent> AbilitySystemComponent;
	//class UAbilitySystemComponent* AbilitySystemComponent;
	//virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void CancelAbilityWithTags(const FGameplayTagContainer CancelWithTags);
	
	UPROPERTY()
	UObkCombatAttributeSet* AttributeSetBase;
	//TObjectPtr<class UObkCombatAttributeSet> AttributeSetBase;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Abilities|Attributes")
	TSubclassOf<class UGameplayEffect> DefaultAttributes;

	//Health
	FDelegateHandle HealthChangedDelegateHandle;
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
	void OnHealthChangedInternal(const FOnAttributeChangeData& Data);
	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities|Attributes")
	void OnHealthChanged(float oldValue, float newValue);
	UFUNCTION(BlueprintPure, Category = "Abilities|Attributes")
	float GetHealth() const;
	UFUNCTION(BlueprintCallable, Category = "Abilities|Attributes")
	void SetHealth(float NewVal);
	UFUNCTION(BlueprintPure, Category = "Abilities|Attributes")
	float GetMaxHealth() const;
};



//Hit boxes related---------------------------------------------------------------------------------------------------


USTRUCT(BlueprintType)
struct FPoCharacterHitBox {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FName SocketName = "hand_r";
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Radius = 25.0f;
};

USTRUCT(BlueprintType, meta = (FullyExpand = true))
struct FPoCharacterHitBoxContainer {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FGameplayTag GameplayTag;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta=(FullyExpand=true))
	TArray <FPoCharacterHitBox> HitBoxArray;
};

