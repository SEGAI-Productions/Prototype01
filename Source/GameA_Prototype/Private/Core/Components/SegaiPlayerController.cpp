// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Components/SegaiPlayerController.h"

void ASegaiPlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);

	if (bHideViewTargetPawnNextFrame)
	{
		AActor* const ViewTargetPawn = PlayerCameraManager ? Cast<AActor>(PlayerCameraManager->GetViewTarget()) : nullptr;
		if (ViewTargetPawn)
		{
			// internal helper func to hide all the components
			auto AddToHiddenComponents = [&OutHiddenComponents](const TInlineComponentArray<UPrimitiveComponent*>& InComponents)
				{
					// add every component and all attached children
					for (UPrimitiveComponent* Comp : InComponents)
					{
						if (Comp->IsRegistered())
						{
							OutHiddenComponents.Add(Comp->GetPrimitiveSceneId());

							for (USceneComponent* AttachedChild : Comp->GetAttachChildren())
							{
								static FName NAME_NoParentAutoHide(TEXT("NoParentAutoHide"));
								UPrimitiveComponent* AttachChildPC = Cast<UPrimitiveComponent>(AttachedChild);
								if (AttachChildPC && AttachChildPC->IsRegistered() && !AttachChildPC->ComponentTags.Contains(NAME_NoParentAutoHide))
								{
									OutHiddenComponents.Add(AttachChildPC->GetPrimitiveSceneId());
								}
							}
						}
					}
				};

			//TODO Solve with an interface.  Gather hidden components or something.
			//TODO Hiding isn't awesome, sometimes you want the effect of a fade out over a proximity, needs to bubble up to designers.

			// hide pawn's components
			TInlineComponentArray<UPrimitiveComponent*> PawnComponents;
			ViewTargetPawn->GetComponents(PawnComponents);
			AddToHiddenComponents(PawnComponents);

			//// hide weapon too
			//if (ViewTargetPawn->CurrentWeapon)
			//{
			//	TInlineComponentArray<UPrimitiveComponent*> WeaponComponents;
			//	ViewTargetPawn->CurrentWeapon->GetComponents(WeaponComponents);
			//	AddToHiddenComponents(WeaponComponents);
			//}
		}

		// we consumed it, reset for next frame
		bHideViewTargetPawnNextFrame = false;
	}
}

void ASegaiPlayerController::OnCameraPenetratingTarget()
{
	bHideViewTargetPawnNextFrame = true;
}