// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Actors/PrototypeBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Core/Camera/GPCameraComponent.h"
#include "InputAction.h"
#include "Core/Components/GPEnhancedInputComponent.h"
#include "Logging/LogMacros.h"
#include "Core/Components/PrototypeAbilitySystemComponent.h"
#include "Core/Abilities/GPGameplayAbilitySet.h"
#include "Core/Abilities/PrototypeGameplayAbility.h"
#include "Core/Attributes/GPPlayerAttributeSet.h"
#include "Core/Attributes/PrototypeAttributeSet.h"
#include "Core/Tags/PrototypeGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffectExtension.h"
#include <Kismet/KismetSystemLibrary.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(PrototypeBaseCharacter)

// Sets default values
APrototypeBaseCharacter::APrototypeBaseCharacter(const class FObjectInitializer& ObjectInitializer)
{
	AbilityCameraMode = nullptr;


	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bAbilitiesInitialized = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Overlap);

	CameraComponent = CreateDefaultSubobject<UGPCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));
	CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &APrototypeBaseCharacter::DetermineCameraMode);

	AbilitySystemComponent = CreateDefaultSubobject<UPrototypeAbilitySystemComponent>(TEXT("Ability System"));

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetIsReplicated(true);
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	}

	Attributes = DefaultAttributes.GetDefaultObject();
}

// Called when the game starts or when spawned
void APrototypeBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(AbilitySystemComponent)) return;

	UPrototypeAttributeSet* prototypeAttributes = Cast<UPrototypeAttributeSet>(Attributes);

	if (prototypeAttributes)
	{
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(prototypeAttributes->GetAttackWeightAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnAttackWeightChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(prototypeAttributes->GetMaxHealthAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnMaxHealthChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(prototypeAttributes->GetHealthAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnHealthChangedInternal);
	}

	UGPPlayerAttributeSet* playerAttributes = Cast<UGPPlayerAttributeSet>(Attributes);

	if (playerAttributes)
	{
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(playerAttributes->GetStrengthAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnStrengthChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(playerAttributes->GetAgilityAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnAgilityChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(playerAttributes->GetVitalityAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnVitalityChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(playerAttributes->GetIntelligenceAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnIntelligenceChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(playerAttributes->GetDexterityAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnDexterityChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(playerAttributes->GetLuckAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnLuckChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(playerAttributes->GetPrecisionAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnPrecisionChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(playerAttributes->GetReflexAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnReflexChangedInternal);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(playerAttributes->GetXPAttribute()).AddUObject(this, &APrototypeBaseCharacter::OnXPChangedInternal);
	}
}

void APrototypeBaseCharacter::Landed(const FHitResult& Hit)
{
}

void APrototypeBaseCharacter::NotifyJumpApex()
{
}

TSubclassOf<UGPCameraMode> APrototypeBaseCharacter::DetermineCameraMode() const
{
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	if (CameraComponent)
	{
		return CameraComponent->DefaultCameraMode;
	}

	return nullptr;
}

UAbilitySystemComponent* APrototypeBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void APrototypeBaseCharacter::AddStartupGameplayAbilities()
{
	SetRole(ROLE_Authority);

	if (GetLocalRole() != ROLE_Authority || bAbilitiesInitialized) return;

	check(AbilitySystemComponent);

	for (TObjectPtr<UGPGameplayAbilitySet>& AbilitySet : GameplayAbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &AbilitySet->GrantedHandles, nullptr);
		}
	}

	// Now Apply Passives
	for (const TSubclassOf<UGameplayEffect>& GameplayEffect : PassiveGameplayEffects)
	{
		// FGameplayEffectContextHandle: Wrapper that allows effect to be polymorphic and replicated over network.
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle NewHandle = 
			AbilitySystemComponent->MakeOutgoingSpec(
				GameplayEffect,
				1,
				EffectContext
			);

		if (NewHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGameplayEffectHandle =
				AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
					*NewHandle.Data.Get(), 
					AbilitySystemComponent
				);
		}
	}

	bAbilitiesInitialized = true;
}

void APrototypeBaseCharacter::AddCharacterAbilityWithInput(TSubclassOf<UPrototypeGameplayAbility> Ability)
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent) return;

	if (Ability)
	{
		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(
				Ability,
				Ability.GetDefaultObject()->GetAbilityLevel(),
				GetTypeHash(Ability.GetDefaultObject()->AbilityInputID = *InputConfig->FindGameplayTagForAbility(Ability, true)),
				this
			));
	}

	UE_LOG(LogTemp, Warning, TEXT("Ability Name : %s"), *Ability.GetDefaultObject()->GetName());
}

void APrototypeBaseCharacter::RemoveCharacterGameplayAbilities()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent) return;
	
	GrantedHandles.TakeFromAbilitySystem(AbilitySystemComponent);

	//TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;

	//for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	//{
	//	if ((Spec.SourceObject == this) && GameplayAbilities.Contains(Spec.Ability->GetClass())) {
	//		AbilitiesToRemove.Add(Spec.Handle);
	//	}
	//}

	//for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
	//{
	//	AbilitySystemComponent->ClearAbility(AbilitiesToRemove[i]);
	//}
}

void APrototypeBaseCharacter::RemoveCharacterGameplayAbility(const FGameplayAbilitySpecHandle& AbilitySpecHandle)
{
	AbilitySystemComponent->ClearAbility(AbilitySpecHandle);
}

void APrototypeBaseCharacter::InitializeAttributes()
{
}

void APrototypeBaseCharacter::InitializeHealth(float health)
{
	UPrototypeAttributeSet* prototypeAttributes = Cast<UPrototypeAttributeSet>(Attributes);
	if (prototypeAttributes)
	{
		prototypeAttributes->InitHealth(health);
		prototypeAttributes->InitMaxHealth(health);
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Attribute Set Not Initialized!"));
}

UEnhancedInputLocalPlayerSubsystem* APrototypeBaseCharacter::GetInputSubsystem() const
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = nullptr;

	// Make sure that we have a valid PlayerController.
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController) 
	{ 
		Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()); 
	}

	return Subsystem;
}

void APrototypeBaseCharacter::CancelAbilityWithTags(const FGameplayTagContainer CancelWithTags) {
	AbilitySystemComponent->CancelAbilities(&CancelWithTags);
}

float APrototypeBaseCharacter::GetHealth() const
{
	UPrototypeAttributeSet* prototypeAttributes = Cast<UPrototypeAttributeSet>(Attributes);
	if (prototypeAttributes)
	{
		return prototypeAttributes->GetHealth();
	}

	UE_LOG(LogTemp, Warning, TEXT("Attribute Set Not Initialized!"));
	return 0.0f;
}

float APrototypeBaseCharacter::GetMaxHealth() const
{
	UPrototypeAttributeSet* prototypeAttributes = Cast<UPrototypeAttributeSet>(Attributes);
	if (prototypeAttributes)
	{
		return prototypeAttributes->GetMaxHealth();
	}

	UE_LOG(LogTemp, Warning, TEXT("Attribute Set Not Initialized!"));
	return 0.0f;
}

void APrototypeBaseCharacter::PossessedBy(AController* NewController)
{
	UE_LOG(LogTemp, Warning, TEXT("Pawn is possessed by: %s"), *NewController->GetHumanReadableName());
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AddStartupGameplayAbilities();
	}

	SetOwner(NewController);
}

// Called every frame
void APrototypeBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APrototypeBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UE_LOG(LogTemp, Warning, TEXT("Setting up player input component."));
	//Make sure to set your input component class in the InputSettings->DefaultClasses
	check(PlayerInputComponent);

	if (InputConfig == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputConfig is null"));
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	for (const FTaggedInputAction& Action : InputConfig->TaggedInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("InputAction bound to InputAction %s"), *Action.InputTag.ToString());

			EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Triggered, this, &APrototypeBaseCharacter::AbilityInputPressed, Action.InputTag);
			EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Completed, this, &APrototypeBaseCharacter::AbilityInputReleased, Action.InputTag);
		}
	}
}

void APrototypeBaseCharacter::SetAbilityCameraMode(TSubclassOf<UGPCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		AbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void APrototypeBaseCharacter::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		AbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}

void APrototypeBaseCharacter::AbilityInputPressed(const FGameplayTag InputTag)
{
	AbilitySystemComponent->AbilityLocalInputPressed(GetTypeHash(InputTag));
}

void APrototypeBaseCharacter::AbilityInputReleased(const FGameplayTag InputTag)
{
	AbilitySystemComponent->AbilityLocalInputReleased(GetTypeHash(InputTag));
}

void APrototypeBaseCharacter::OnPrimaryAction()
{
	// Trigger the OnItemUsed Event
	//OnUseItem.Broadcast();
}

void APrototypeBaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AbilitySystemComponent || InputComponent) return;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	const FGameplayAbilityInputBinds Binds(
		"Confirm",
		"Cancel",
		"PrototypeAbilityInputID",
		static_cast<int32>(EAbilityInputID::Confirm),
		static_cast<int32>(EAbilityInputID::Cancel)
	);

	AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, Binds);
}

void APrototypeBaseCharacter::HandleDamage(float DamageAmount, const FHitResult& HitInfo, const FGameplayTagContainer& DamageTags, APrototypeBaseCharacter* InstigatorCharacter, AActor* DamageOwner)
{
	OnDamaged(DamageAmount, HitInfo, DamageTags, InstigatorCharacter, DamageOwner);
}

void APrototypeBaseCharacter::HandleAttackWeightChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnAttackWeightChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleMaxHealthChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnMaxHealthChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleHealthChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnHealthChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleStrengthChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnStrengthChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleAgilityChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnAgilityhChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleVitalityChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnVitalityChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleIntelligenceChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnIntelligenceChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleDexterityChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnDexterityChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleLuckChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnLuckChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandlePrecisionChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnPrecisionChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleReflexChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnReflexChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::HandleXPChanged(float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnXPChanged(DeltaValue, EventTags);
	}
}

void APrototypeBaseCharacter::OnAttackWeightChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnAttackWeightChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnMaxHealthChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnMaxHealthChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnHealthChangedInternal(const FOnAttributeChangeData& Data) 
{
	OnHealthChanged1(Data.OldValue, Data.NewValue);
}

void APrototypeBaseCharacter::OnStrengthChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnStrengthChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnAgilityChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnAgilityhChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnVitalityChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnVitalityChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnIntelligenceChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnIntelligenceChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnDexterityChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnDexterityChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnLuckChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnLuckChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnPrecisionChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnPrecisionChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnReflexChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnReflexChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::OnXPChangedInternal(const FOnAttributeChangeData& Data)
{
	const FGameplayEffectModCallbackData& data = *Data.GEModData;
	const FGameplayTagContainer& SourceTags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	OnXPChanged(Data.OldValue, SourceTags);
}

void APrototypeBaseCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	if (Controller != nullptr)
	{
		const FVector2D MoveValue = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (MoveValue.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, MoveValue.X);
		}

		if (MoveValue.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, MoveValue.Y);
		}
	}
}

void APrototypeBaseCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	if (Controller != nullptr)
	{
		const FVector2D LookValue = InputActionValue.Get<FVector2D>();

		if (LookValue.X != 0.0f)
		{
			TurnAtRate(LookValue.X);
		}

		if (LookValue.Y != 0.0f)
		{
			LookUpAtRate(LookValue.Y);
		}
	}
}

void APrototypeBaseCharacter::Input_Fire(const EAbilityInputID AbilityInputTag)
{
	OnPrimaryAction();
}

void APrototypeBaseCharacter::EquipWeapon()
{
	for (const FGPEquipmentActorToSpawn& SpawnInfo : EquipmentToSpawn)
	{
		AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, this);
		NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
		NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
		NewActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket); 
		DestroyEquipmentActors();
		SpawnedEquipment = NewActor;
	}
}

void APrototypeBaseCharacter::DestroyEquipmentActors()
{
	if (SpawnedEquipment)
	{
		SpawnedEquipment->Destroy();
	}
}

APawn* APrototypeBaseCharacter::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}

void APrototypeBaseCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void APrototypeBaseCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}