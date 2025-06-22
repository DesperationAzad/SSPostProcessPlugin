// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
public class SSPostProcessPlugin : ModuleRules
{
	public SSPostProcessPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				//"Renderer",
				// ... add public include paths required here ...
			}
		);
				
		var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(EngineDir, "Source/Runtime/Renderer/Private"),
				// ... add other private include paths required here ...
				Path.Combine(EngineDir, "Source/Runtime/Renderer/Private/PostProcess"),
				Path.Combine(EngineDir, "Source/Runtime/RenderCore/Private"),
				Path.Combine(EngineDir, "Source/Runtime/RHI/Private"),
				Path.Combine(EngineDir, "Source/Runtime/Engine/Classes/Engine"),
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RenderCore",
				"Renderer",
				"RHI",
				"Projects", 
				// ... add other public dependencies that you statically link with here ...
				// 添加新依赖
                "ImageWrapper",
                "ImageCore",
            }
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
				// 添加新依赖
                "RenderCore",
                "Renderer",
                "RHI",
				"RHICore",
            }
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}