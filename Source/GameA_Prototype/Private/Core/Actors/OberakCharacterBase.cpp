#include "Core/Actors/OberakCharacterBase.h"
#include "Core/Components/OberakAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"

AOberakCharacterBase::AOberakCharacterBase(const class FObjectInitializer& ObjectInitializer) {
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UOberakAbilitySystemComponent>(TEXT("Ability System Component"));
	AttributeSetBase = CreateDefaultSubobject<UObkCombatAttributeSet>(TEXT("AttributeSetBase"));
}
void AOberakCharacterBase::BeginPlay() {
	Super::BeginPlay();
	if (IsValid(AbilitySystemComponent)) {
		//FGameplayAttribute attribute = AttributeSetBase->GetHealthAttribute();
		//FOnGameplayAttributeValueChange delegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(attribute);
		//delegate.AddUObject(this, &AOberakCharacterBase::OnHealthChangedInternal);

		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).AddUObject(this, &AOberakCharacterBase::HealthChanged);
	}
}
void AOberakCharacterBase::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	InitializeAttributes();
}
UAbilitySystemComponent* AOberakCharacterBase::GetAbilitySystemComponent() const {
	return AbilitySystemComponent;
}
void AOberakCharacterBase::CancelAbilityWithTags(const FGameplayTagContainer CancelWithTags) {
	AbilitySystemComponent->CancelAbilities(&CancelWithTags);
}



//Health
void AOberakCharacterBase::InitializeAttributes() {
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, 1.0f, EffectContext);
	if (NewHandle.IsValid()) {
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent.Get());
	}
}
void AOberakCharacterBase::OnHealthChangedInternal(const FOnAttributeChangeData& Data) {
	OnHealthChanged(Data.OldValue, Data.NewValue);
}
float AOberakCharacterBase::GetHealth() const {
	//if (IsValid(AttributeSet)) { return AttributeSet->GetHealth(); }
	//if (AttributeSetBase) { return AttributeSetBase->GetHealth(); }
	if (AttributeSetBase) {
		const UAttributeSet* baseSet = GetAbilitySystemComponent()->GetAttributeSet(UObkCombatAttributeSet::StaticClass());
		const UObkCombatAttributeSet* attributeSet = Cast<UObkCombatAttributeSet>(baseSet);
		return attributeSet->GetHealth();
	}
	return -1.0f;
}
void AOberakCharacterBase::SetHealth(float NewVal) {
	//if (IsValid(AttributeSet)) { AttributeSet->SetHealth(NewVal); }
	if (AttributeSetBase) { AttributeSetBase->SetHealth(NewVal); }
}
float AOberakCharacterBase::GetMaxHealth() const {
	//if (IsValid(AttributeSet)) { return AttributeSet->GetMaxHealth(); }
	if (AttributeSetBase) { return AttributeSetBase->GetMaxHealth(); }
	return -1.0f;
}
void AOberakCharacterBase::HealthChanged(const FOnAttributeChangeData& Data) {
	OnHealthChangedInternal(Data);
}