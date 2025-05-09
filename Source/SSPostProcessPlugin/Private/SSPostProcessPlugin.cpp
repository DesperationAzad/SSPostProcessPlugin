// Copyright Epic Games, Inc. All Rights Reserved.

#include "SSPostProcessPlugin.h"

#include "Interfaces/IPluginManager.h"


#define LOCTEXT_NAMESPACE "FSSPostProcessPluginModule"

void FSSPostProcessPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// 创建FSampleRender实例
	//SampleRenderExtension = MakeShared<FSampleRender>();
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("SSPostProcessPlugin"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/SSPostProcessPlugin"), PluginShaderDir);
}

void FSSPostProcessPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSSPostProcessPluginModule, SSPostProcessPlugin)