// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Camera/GPImpactCameraShake.h"

UGPImpactCameraShake::UGPImpactCameraShake()
{
	OscillationDuration = .5f;
	OscillationBlendInTime = .1f;
	OscillationBlendOutTime = .2f;

	RotOscillation.Pitch.Amplitude = 3.0f;
	RotOscillation.Pitch.Frequency = 3.0f;
	RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
	RotOscillation.Pitch.Waveform = EOscillatorWaveform::PerlinNoise;

	LocOscillation.Y.Amplitude = 5.0f;
	LocOscillation.Y.Frequency = 3.0f;
	LocOscillation.Y.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
	LocOscillation.Y.Waveform = EOscillatorWaveform::SineWave;

	LocOscillation.Z.Amplitude = 5.0f;
	LocOscillation.Z.Frequency = 3.0f;
	LocOscillation.Z.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
	LocOscillation.Z.Waveform = EOscillatorWaveform::SineWave;
}

void UGPImpactCameraShake::SetOscillationDuration(float Duration)
{
	OscillationDuration = Duration;
}
