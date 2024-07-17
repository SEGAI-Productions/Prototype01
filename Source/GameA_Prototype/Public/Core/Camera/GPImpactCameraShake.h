// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shakes/LegacyCameraShake.h"
#include "GPImpactCameraShake.generated.h"

/**
 * 
 */
UCLASS()
class GAMEA_PROTOTYPE_API UGPImpactCameraShake : public ULegacyCameraShake
{
	GENERATED_BODY()
	
public:
	UGPImpactCameraShake();

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings")
	//float Duration = .5f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings")
	//float BlendInTime = .1f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings")
	//float BlendOutTime = .2f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Rotation")
	//float PitchAmplitude = 3.0f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Rotation")
	//float PitchFrequency = 3.0f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Rotation")
	//TEnumAsByte<EInitialOscillatorOffset> PitchInitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Rotation")
	//EOscillatorWaveform PitchWaveform = EOscillatorWaveform::PerlinNoise;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Location | Y")
	//float YLocationAmplitude = 5.0f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Location | Y")
	//float YLocationFrequency = 3.0f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Location | Y")
	//TEnumAsByte<EInitialOscillatorOffset> YLocationInitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Location | Y")
	//EOscillatorWaveform YLocationWaveform = EOscillatorWaveform::SineWave;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Location | Z")
	//float ZLocationAmplitude = 5.0f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Location | Z")
	//float ZLocationFrequency = 3.0f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Location | Z")
	//TEnumAsByte<EInitialOscillatorOffset> ZLocationInitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OscillationSettings | Location | Z")
	//EOscillatorWaveform ZLocationWaveform = EOscillatorWaveform::SineWave;
};
