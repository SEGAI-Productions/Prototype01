// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GPAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UGPAnimNotify : public UAnimNotify
{
    GENERATED_BODY()

public:
    UGPAnimNotify();

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    void DetectTrackInformation(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    UAnimNotifyState* GetNotifyStatesOnTrack() const;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float GateTiming;

protected:

    UPROPERTY()
    UAnimNotifyState* NotifyStateOnTrack;
};
