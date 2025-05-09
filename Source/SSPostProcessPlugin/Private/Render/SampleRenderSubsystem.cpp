
// Subsystem to keep custom SceneViewExtension alive

#include "Render/SampleRenderSubsystem.h"
#include "Render/CustomSceneViewExtension.h"
#include "SceneViewExtension.h"

void USampleRenderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	CustomSceneViewExtension = FSceneViewExtensions::NewExtension<FCustomSceneViewExtension>();
	UE_LOG(LogTemp, Log, TEXT("SceneViewExtensionTemplate: Subsystem initialized & SceneViewExtension created"));
}

void USampleRenderSubsystem::Deinitialize()
{
	{
		CustomSceneViewExtension->IsActiveThisFrameFunctions.Empty();

		FSceneViewExtensionIsActiveFunctor IsActiveFunctor;

		IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
		{
			return TOptional<bool>(false);
		};

		CustomSceneViewExtension->IsActiveThisFrameFunctions.Add(IsActiveFunctor);
	}

	CustomSceneViewExtension.Reset();
	CustomSceneViewExtension = nullptr;
}
