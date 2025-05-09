// Custom SceneViewExtension implementation

#pragma once

#include "CoreMinimal.h"
#include "RenderGraphUtils.h"
#include "SceneViewExtension.h"
#include "PostProcess/PostProcessing.h"
#include "PostProcess/PostProcessMaterial.h"
#include "SceneTextureParameters.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
#include "DataDrivenShaderPlatformInfo.h"
#endif

class FCustomSceneViewExtension : public FSceneViewExtensionBase
{
public:
	FCustomSceneViewExtension(const FAutoRegister& AutoRegister);

public:
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override
	{
	};

	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override
	{
	};

	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override
	{
	};

	// See SceneViewExtension.h for hooks to different stages of rendering
	// f.ex. PrePostProcessPass_RenderThread happens just when rendering is finished but PostProcessing hasn't started yet

	// This is the method to hook into PostProcessing pass
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5
	virtual void SubscribeToPostProcessingPass(EPostProcessingPass PassId, const FSceneView& View, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled);
#else
	virtual void SubscribeToPostProcessingPass(EPostProcessingPass PassId,
	                                           FAfterPassCallbackDelegateArray& InOutPassCallbacks,
	                                           bool bIsPassEnabled);
#endif

	// This is our actual processing function
	FScreenPassTexture SamplePostProcessing(FRDGBuilder& GraphBuilder, const FSceneView& SceneView,
	                                        const FPostProcessMaterialInputs& Inputs);
	FScreenPassTexture ScreenspaceReflectionPostProcessing(FRDGBuilder& GraphBuilder, const FSceneView& SceneView,
	                                                       const FPostProcessMaterialInputs& Inputs);

public:

private:
};

// Shader declarations

// Struct to include common parameters, useful when doing multiple shaders
BEGIN_SHADER_PARAMETER_STRUCT(FCommonShaderParameters,)
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, ViewUniformBuffer)
END_SHADER_PARAMETER_STRUCT()

// Sample 拉普拉斯描边 Post Process Shader
//TODO: By GYP
class FSamplePS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FSamplePS, Global);

	SHADER_USE_PARAMETER_STRUCT(FSamplePS, FGlobalShader);

	// 内联着色器参数定义。按照约定使用FParameters名称。
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)

		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		//FViewUniformShaderParameters.View
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColorTexture)

		SHADER_PARAMETER_SAMPLER(SamplerState, SceneColorSampler)

		RENDER_TARGET_BINDING_SLOTS()

	END_SHADER_PARAMETER_STRUCT()
};

// SSR Post Process Shader
//TODO: By GYP
class FScreenspaceReflectionPS : public FGlobalShader
{
public:
public:
	DECLARE_SHADER_TYPE(FScreenspaceReflectionPS, Global);

	SHADER_USE_PARAMETER_STRUCT(FScreenspaceReflectionPS, FGlobalShader);

	// 内联着色器参数定义。按照约定使用FParameters名称。
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)

		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		//FViewUniformShaderParameters.View
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColorTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SceneColorSampler)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HZB)
		SHADER_PARAMETER_SAMPLER(SamplerState, HZBSampler)
		SHADER_PARAMETER(FVector4f, HZBUvFactorAndInvFactor)
		SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureParameters, SceneTextures)

		RENDER_TARGET_BINDING_SLOTS()

	END_SHADER_PARAMETER_STRUCT()
};

