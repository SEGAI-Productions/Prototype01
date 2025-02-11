// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Camera/GPCameraMode.h"
#include "Core/Camera/SegaiCameraManager.h"
#include "Curves/CurveVector.h"
#include "Core/Camera/GPCameraMode_Dynamic.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Core/Camera/GPCameraComponent.h"
#include <DrawDebugHelpers.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(GPCameraMode)


//////////////////////////////////////////////////////////////////////////
// FGPCameraModeView
//////////////////////////////////////////////////////////////////////////
FGPCameraModeView::FGPCameraModeView()
	: Location(ForceInit)
	, Rotation(ForceInit)
	, ControlRotation(ForceInit)
	, FieldOfView(GP_CAMERA_DEFAULT_FOV)
{
}

void FGPCameraModeView::Blend(const FGPCameraModeView& Other, float OtherWeight)
{
	if (OtherWeight <= 0.0f)
	{
		return;
	}
	else if (OtherWeight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, OtherWeight);

	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (OtherWeight * DeltaRotation);

	const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
	ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}


//////////////////////////////////////////////////////////////////////////
// UGPCameraMode
//////////////////////////////////////////////////////////////////////////
UGPCameraMode::UGPCameraMode()
{
	FieldOfView = GP_CAMERA_DEFAULT_FOV;
	ViewPitchMin = GP_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = GP_CAMERA_DEFAULT_PITCH_MAX;

	BlendTime = 0.5f;
	BlendFunction = EGPCameraModeBlendFunction::EaseOut;
	BlendExponent = 4.0f;
	BlendAlpha = 1.0f;
	BlendWeight = 1.0f;
}

void UGPCameraMode::SetFocusList(const TArray<TWeakObjectPtr<AActor>>& NewList)
{
	//UE_LOG(LogTemp, Warning, TEXT("%s: %p"), *GetName(), this);
	FocusActorList.Reset();
	FocusActorList = NewList;
}

UGPCameraComponent* UGPCameraMode::GetGPCameraComponent() const
{
	return CastChecked<UGPCameraComponent>(GetOuter());
}

UWorld* UGPCameraMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UGPCameraMode::GetTargetActor() const
{
	const UGPCameraComponent* GPCameraComponent = GetGPCameraComponent();

	return GPCameraComponent->GetTargetActor();
}

void UGPCameraMode::UpdateCameraMode(float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
	DrawPersistentDebug();
}

void UGPCameraMode::SetFocusActor(AActor* NewFocusActor)
{
	if (!NewFocusActor) return;
	
	if (NewFocusActor != FocusActor)
	{
		FocusActor = NewFocusActor;
	}
}

void UGPCameraMode::FocusSocketByName(FName FocusSocket)
{
	FocusSocketName = FocusSocket;
}

void UGPCameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
	const float InvExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

	switch (BlendFunction)
	{
	case EGPCameraModeBlendFunction::Linear:
		BlendAlpha = BlendWeight;
		break;

	case EGPCameraModeBlendFunction::EaseIn:
		BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EGPCameraModeBlendFunction::EaseOut:
		BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EGPCameraModeBlendFunction::EaseInOut:
		BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	default:
		checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void UGPCameraMode::DrawDebug(UCanvas* Canvas) const
{
	{
		check(Canvas);

		FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

		DisplayDebugManager.SetDrawColor(FColor::White);
		DisplayDebugManager.DrawString(FString::Printf(TEXT("      GPCameraMode: %s (%f)"), *GetName(), BlendWeight));
	}
}

uint8 UGPCameraMode::CalculateOptimalCameraOrientation(float DeltaTime, const TArray<TWeakObjectPtr<AActor>>& Actors, FVector& OutCenterPoint, float& OutOptimalDistance, const float& FOV)
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	FVector Center = TargetActor->GetActorLocation();
	FBox Bounds(EForceInit::ForceInit); // Initialize bounding box.
	Bounds += Center; // Add the TargetActor's location first

	uint8 TargetCount = 0;
	for (const TWeakObjectPtr<AActor>& Enemy : Actors)
	{
		if (Enemy.IsValid())
		{
			Bounds += Enemy->GetActorLocation();
			TargetCount += 1;
		}
	}

	// Expand bounds slightly for padding
	Bounds = Bounds.ExpandBy(200.0f);

	Center = Bounds.GetCenter();
	Center.Z = GetPivotLocation().Z;
	Bounds = Bounds.MoveTo(Center);

	FVector Extents = FVector::Zero();
	Bounds.GetCenterAndExtents(OutCenterPoint, Extents);

	FVector RotationVector = View.Rotation.Vector();
	RotationVector.Normalize();
	FVector2D RotationVector2D(RotationVector.X, RotationVector.Y);

	FVector2D Offset2D = RotationVector2D * (OutOptimalDistance / 4) * -1;
	FVector Offset(Offset2D, 0);

	Bounds = Bounds.MoveTo(Center + Offset);

	OutOptimalDistance = CalculateOptimalCameraDistance(Bounds, Center, FOV); // FOV (Field Of View)

	return TargetCount;
}

float UGPCameraMode::CalculateOptimalCameraDistance(const FBox& Box, const FVector& Center, const float& FOV) const
{
	float MaxDistance = FVector::Dist(Box.Max, Center);
	return MaxDistance / FMath::Tan(FMath::DegreesToRadians(FOV / 2.0f));
}

FVector UGPCameraMode::GetPivotLocation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// Height adjustments for characters to account for crouching.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			check(TargetCharacterCDO);

			const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
			check(CapsuleComp);

			const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
			check(CapsuleCompCDO);

			const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
			const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
			const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;

			return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
		}

		return TargetPawn->GetPawnViewLocation();
	}

	return TargetActor->GetActorLocation();
}

FRotator UGPCameraMode::GetPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetViewRotation();
	}

	return TargetActor->GetActorRotation();
}

void UGPCameraMode::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = PivotRotation;
	View.FieldOfView = FieldOfView;
}

void UGPCameraMode::UpdateBlending(float DeltaTime)
{
	if (BlendTime > 0.0f)
	{
		BlendAlpha += (DeltaTime / BlendTime);
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

	switch (BlendFunction)
	{
	case EGPCameraModeBlendFunction::Linear:
		BlendWeight = BlendAlpha;
		break;

	case EGPCameraModeBlendFunction::EaseIn:
		BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EGPCameraModeBlendFunction::EaseOut:
		BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EGPCameraModeBlendFunction::EaseInOut:
		BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	default:
		checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void UGPCameraMode::UpdateCameraLag(float DeltaTime, FVector& PivotLocation, FRotator& PivotRotation)
{
#pragma region CameraLag

	FRotator DesiredRot = PivotRotation; //I think viewrotation should go here.

	// Apply 'lag' to rotation if desired
	if (bEnableCameraRotationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraRotationLagSpeed > 0.f)
		{
			const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
			FRotator LerpTarget = PreviousDesiredRot;
			float RemainingTime = DeltaTime;
			while (RemainingTime > UE_KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(LerpTarget), LerpAmount, CameraRotationLagSpeed));
				PreviousDesiredRot = DesiredRot;
			}
		}
		else
		{
			DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(DesiredRot), DeltaTime, CameraRotationLagSpeed));
		}
		//View.ControlRotation = DesiredRot; //This might help smooth the rotation if we're not aligning the pawn rotation to the control rotation.
	}
	PreviousDesiredRot = DesiredRot;
	PivotRotation = DesiredRot;

	// We lag the target, not the actual camera position, so rotating the camera around does not have lag
	FVector DesiredLoc = PivotLocation;
	if (bEnableCameraLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
		{
			const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
			FVector LerpTarget = PreviousDesiredLoc;

			float RemainingTime = DeltaTime;
			while (RemainingTime > UE_KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmMovementStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
				PreviousDesiredLoc = DesiredLoc;
			}
		}
		else
		{
			DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (CameraLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLoc - PivotLocation;
			if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
			{
				DesiredLoc = PivotLocation + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
				bClampedDist = true;
			}
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (bDrawDebugLagMarkers)
		{
			DrawDebugSphere(GetWorld(), PivotLocation, 5.f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLoc, 5.f, 8, FColor::Yellow);

			const FVector ToOrigin = PivotLocation - DesiredLoc;
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc, DesiredLoc + ToOrigin * 0.5f, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc + ToOrigin * 0.5f, PivotLocation, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
		}
#endif
	}

	PreviousDesiredLoc = DesiredLoc;
	PivotLocation = DesiredLoc;

#pragma endregion
}


//////////////////////////////////////////////////////////////////////////
// UGPCameraModeStack
//////////////////////////////////////////////////////////////////////////
UGPCameraModeStack::UGPCameraModeStack()
{
	bIsActive = true;
}

void UGPCameraModeStack::ActivateStack()
{
	if (!bIsActive)
	{
		bIsActive = true;

		// Notify camera modes that they are being activated.
		for (UGPCameraMode* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnActivation();
		}
	}
}

void UGPCameraModeStack::DeactivateStack()
{
	if (bIsActive)
	{
		bIsActive = false;

		// Notify camera modes that they are being deactivated.
		for (UGPCameraMode* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnDeactivation();
		}
	}
}

void UGPCameraModeStack::UpdateCameraFocusActors(TSubclassOf<UGPCameraMode> CameraModeClass, const TArray<TWeakObjectPtr<AActor>>& NewActorList)
{
	if (!CameraModeClass)
	{
		return;
	}

	UGPCameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);
	check(CameraMode);

	CameraMode->SetFocusList(NewActorList);
}

void UGPCameraModeStack::PushCameraMode(TSubclassOf<UGPCameraMode> CameraModeClass)
{
	if (!CameraModeClass)
	{
		return;
	}

	UGPCameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);
	check(CameraMode);

	int32 StackSize = CameraModeStack.Num();

	if ((StackSize > 0) && (CameraModeStack[0] == CameraMode))
	{
		// Already top of stack.
		return;
	}

	// See if it's already in the stack and remove it.
	// Figure out how much it was contributing to the stack.
	int32 ExistingStackIndex = INDEX_NONE;
	float ExistingStackContribution = 1.0f;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		if (CameraModeStack[StackIndex] == CameraMode)
		{
			ExistingStackIndex = StackIndex;
			ExistingStackContribution *= CameraMode->GetBlendWeight();
			break;
		}
		else
		{
			ExistingStackContribution *= (1.0f - CameraModeStack[StackIndex]->GetBlendWeight());
		}
	}

	if (ExistingStackIndex != INDEX_NONE)
	{
		CameraModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	}
	else
	{
		ExistingStackContribution = 0.0f;
	}

	// Decide what initial weight to start with.
	const bool bShouldBlend = ((CameraMode->GetBlendTime() > 0.0f) && (StackSize > 0));
	const float BlendWeight = (bShouldBlend ? ExistingStackContribution : 1.0f);

	CameraMode->SetBlendWeight(BlendWeight);

	// Add new entry to top of stack.
	CameraModeStack.Insert(CameraMode, 0);

	// Make sure stack bottom is always weighted 100%.
	CameraModeStack.Last()->SetBlendWeight(1.0f);

	// Let the camera mode know if it's being added to the stack.
	if (ExistingStackIndex == INDEX_NONE)
	{
		CameraMode->OnActivation();
	}
}

bool UGPCameraModeStack::EvaluateStack(float DeltaTime, FGPCameraModeView& OutCameraModeView)
{
	if (!bIsActive)
	{
		return false;
	}

	UpdateStack(DeltaTime);
	BlendStack(OutCameraModeView);

	return true;
}

UGPCameraMode* UGPCameraModeStack::GetCameraModeInstance(TSubclassOf<UGPCameraMode> CameraModeClass)
{
	check(CameraModeClass);

	// First see if we already created one.
	for (UGPCameraMode* CameraMode : CameraModeInstances)
	{
		if ((CameraMode != nullptr) && (CameraMode->GetClass() == CameraModeClass))
		{
			return CameraMode;
		}
	}

	// Not found, so we need to create it.
	UGPCameraMode* NewCameraMode = NewObject<UGPCameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
	check(NewCameraMode);

	CameraModeInstances.Add(NewCameraMode);

	return NewCameraMode;
}

void UGPCameraModeStack::UpdateStack(float DeltaTime)
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return;
	}

	int32 RemoveCount = 0;
	int32 RemoveIndex = INDEX_NONE;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		UGPCameraMode* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		CameraMode->UpdateCameraMode(DeltaTime);

		if (CameraMode->GetBlendWeight() >= 1.0f)
		{
			// Everything below this mode is now irrelevant and can be removed.
			RemoveIndex = (StackIndex + 1);
			RemoveCount = (StackSize - RemoveIndex);
			break;
		}
	}

	if (RemoveCount > 0)
	{
		// Let the camera modes know they being removed from the stack.
		for (int32 StackIndex = RemoveIndex; StackIndex < StackSize; ++StackIndex)
		{
			UGPCameraMode* CameraMode = CameraModeStack[StackIndex];
			check(CameraMode);

			CameraMode->OnDeactivation();
		}

		CameraModeStack.RemoveAt(RemoveIndex, RemoveCount);
	}
}

void UGPCameraModeStack::BlendStack(FGPCameraModeView& OutCameraModeView) const
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return;
	}

	// Start at the bottom and blend up the stack
	const UGPCameraMode* CameraMode = CameraModeStack[StackSize - 1];
	check(CameraMode);

	OutCameraModeView = CameraMode->GetCameraModeView();

	for (int32 StackIndex = (StackSize - 2); StackIndex >= 0; --StackIndex)
	{
		CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
	}
}

void UGPCameraModeStack::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString(TEXT("   --- Camera Modes (Begin) ---")));

	for (const UGPCameraMode* CameraMode : CameraModeStack)
	{
		check(CameraMode);
		CameraMode->DrawDebug(Canvas);
	}

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   --- Camera Modes (End) ---")));
}

void UGPCameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	if (CameraModeStack.Num() == 0)
	{
		OutWeightOfTopLayer = 1.0f;
		OutTagOfTopLayer = FGameplayTag();
		return;
	}
	else
	{
		UGPCameraMode* TopEntry = CameraModeStack.Last();
		check(TopEntry);
		OutWeightOfTopLayer = TopEntry->GetBlendWeight();
		OutTagOfTopLayer = TopEntry->GetCameraTypeTag();
	}
}

void UGPCameraModeStack::SetFocusObject(TSubclassOf<UGPCameraMode> CameraModeClass, AActor* NewFocusObject)
{
	UGPCameraMode* cameraMode = GetCameraModeInstance(CameraModeClass);

	if (cameraMode)
	{
		cameraMode->SetFocusActor(NewFocusObject);
	}
}

void UGPCameraModeStack::SetDynamicOffsetCurve(TSubclassOf<UGPCameraMode> CameraModeClass, TObjectPtr<UCurveVector> DynamicOffset)
{
	UGPCameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);

	if (CameraMode)
	{
		if (UGPCameraMode_Dynamic* DynamicCameraMode = Cast<UGPCameraMode_Dynamic>(CameraMode))
		{
			if (DynamicOffset)
			{
				DynamicCameraMode->SetDynamicOffsetCurve(DynamicOffset);
			}
		}
	}
}

void UGPCameraModeStack::FocusSocketByName(TSubclassOf<UGPCameraMode> CameraModeClass, FName FocusSocket)
{
	UGPCameraMode* cameraMode = GetCameraModeInstance(CameraModeClass);

	cameraMode->FocusSocketByName(FocusSocket);
}
