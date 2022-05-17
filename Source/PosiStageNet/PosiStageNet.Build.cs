// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PosiStageNet : ModuleRules
{
    public PosiStageNet(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] 
			{
			});
				
		
		PrivateIncludePaths.AddRange(
			new string[] 
			{
			});
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects",
				"Engine",
				"CoreUObject"
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Networking",
				"Sockets",
			});
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			});
	}
}
