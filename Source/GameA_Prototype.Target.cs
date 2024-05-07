// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class GameA_PrototypeTarget : TargetRules
{
	public GameA_PrototypeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

		ExtraModuleNames.AddRange( new string[] { "GameA_Prototype" } );
	}
}
