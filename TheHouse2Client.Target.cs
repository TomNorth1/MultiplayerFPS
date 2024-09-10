// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TheHouse2ClientTarget : TargetRules
{
	public TheHouse2ClientTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		
		bUsesSteam = true;
		bUseLoggingInShipping = true;
		
		GlobalDefinitions.Add("UE4_PROJECT_STEAMGAMEDIR=\"Spacewar\"");
		GlobalDefinitions.Add("UE4_PROJECT_STEAMSHIPPING=480");
		
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "TheHouse2" } );
	}
}
