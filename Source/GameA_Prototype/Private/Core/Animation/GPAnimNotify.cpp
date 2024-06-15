// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Animation/GPAnimNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Logging/LogMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GPAnimNotify)

UGPAnimNotify::UGPAnimNotify()
{
}

void UGPAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	DetectTrackInformation(MeshComp, Animation, EventReference);
}

UAnimNotifyState* UGPAnimNotify::GetNotifyStatesOnTrack() const
{
    return NotifyStateOnTrack;
}

void UGPAnimNotify::DetectTrackInformation(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    //UE_LOG(LogTemp, Warning, TEXT("Detecting Track Information"));

    const FAnimNotifyEvent* notify = EventReference.GetNotify();

    // Log the trigger time of the NotifyEvent
    UE_LOG(LogTemp, Warning, TEXT("Trigger Time: %f"), notify->GetTriggerTime());

    UAnimMontage* Montage = Cast<UAnimMontage>(Animation);

    if (Montage)
    {
        // Log the class name of the Montage for debugging purposes
        UE_LOG(LogTemp, Warning, TEXT("Attributes class: %s"), *Montage->GetName());

        // Validate that the Montage has SlotAnimTracks
        if (Montage->SlotAnimTracks.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("No SlotAnimTracks found in Montage."));
            return;
        }

#if WITH_EDITORONLY_DATA
        UE_LOG(LogTemp, Warning, TEXT("Num Slot Anim Tracks: %d"), Montage->AnimNotifyTracks.Num());

        TArray<FAnimNotifyEvent*> AnimNotifyEvents = Montage->AnimNotifyTracks[notify->TrackIndex].Notifies;

        for (const FAnimNotifyEvent* NotifyEvent : AnimNotifyEvents)
        {
            if (NotifyEvent && NotifyEvent->Notify != this)
            {
                if (UAnimNotifyState* DefaultObject = Cast<UAnimNotifyState>(NotifyEvent->NotifyStateClass.Get()))
                {
                    // Calculate the percentage
                    float NotifyStateStartTime = NotifyEvent->GetTriggerTime();
                    float NotifyStateDuration = NotifyEvent->GetDuration();
                    GateTiming = ((notify->GetTriggerTime() - NotifyStateStartTime) / NotifyStateDuration);
                    NotifyStateOnTrack = DefaultObject;
                }

                break;
            }
        }
#endif
    }
}
