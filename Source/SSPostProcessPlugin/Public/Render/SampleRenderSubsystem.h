
// Subsystem to keep custom SceneViewExtension alive

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "SampleRenderSubsystem.generated.h"

UCLASS()
class USampleRenderSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	TSharedPtr<class FCustomSceneViewExtension, ESPMode::ThreadSafe> GetCustomSceneViewExtension() const 
	{
		return CustomSceneViewExtension;
	}

private:
	TSharedPtr<class FCustomSceneViewExtension, ESPMode::ThreadSafe> CustomSceneViewExtension;
};