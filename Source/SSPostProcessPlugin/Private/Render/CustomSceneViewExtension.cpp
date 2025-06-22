// Custom SceneViewExtension implementation

#include "Render/CustomSceneViewExtension.h"

#include "ProfilingDebugging/RealtimeGPUProfiler.h"

#include "RenderUtils.h"
#include "SystemTextures.h"
#include "ShaderParameterStruct.h"

DECLARE_GPU_STAT(GYPScreenspaceReflection);
//DECLARE_GPU_STAT(SampleSSAO);





IMPLEMENT_GLOBAL_SHADER(FSamplePS, "/SSPostProcessPlugin/Private/SampleRenderShader.usf", "MainPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FScreenspaceReflectionPS, "/SSPostProcessPlugin/Private/SampleScreenspaceReflection.usf", "MainPS",
                        SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FSampleSSAOPS, "/SSPostProcessPlugin/Private/SampleScreenspaceAmbientOcclusion.usf", "MainPS", SF_Pixel);

namespace
{
	TAutoConsoleVariable<int32> CVarShaderOn(
		TEXT("r.SampleSSR"),
		0,
		TEXT("Enable SampleSSR \n")
		TEXT(" 0: OFF;")
		TEXT(" 1: ON."),
		ECVF_RenderThreadSafe);
	TAutoConsoleVariable<int32> CVarShaderOnSample(
		TEXT("r.SamplePostProcessing"),
		0,
		TEXT("Enable SamplePostProcessing Custom SceneViewExtension implementation\n")
		TEXT(" 0: OFF;")
		TEXT(" 1: ON."),
		ECVF_RenderThreadSafe);

	TAutoConsoleVariable<int32> CVarSSAOOn(
		TEXT("r.SampleSSAO"),
		0,
		TEXT("Enable SSAO\n")
		TEXT(" 0: OFF;")
		TEXT(" 1: ON."),
		ECVF_RenderThreadSafe);
}


FCustomSceneViewExtension::FCustomSceneViewExtension(const FAutoRegister& AutoRegister) : FSceneViewExtensionBase(
	AutoRegister)
{
	UE_LOG(LogTemp, Log, TEXT("SceneViewExtensionTemplate: Custom SceneViewExtension registered"));
}


void FCustomSceneViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass PassId,
                                                              FAfterPassCallbackDelegateArray& InOutPassCallbacks,
                                                              bool bIsPassEnabled)
{
	// Define to what Post Processing stage to hook the SceneViewExtension into. See SceneViewExtension.h and PostProcessing.cpp for more info
	if (PassId == EPostProcessingPass::MotionBlur)
	{

		InOutPassCallbacks.Add(
			FAfterPassCallbackDelegate::CreateRaw(this, &FCustomSceneViewExtension::SamplePostProcessing));
		InOutPassCallbacks.Add(
			FAfterPassCallbackDelegate::CreateRaw(
				this, &FCustomSceneViewExtension::ScreenspaceReflectionPostProcessing));
		InOutPassCallbacks.Add(
			FAfterPassCallbackDelegate::CreateRaw(this, &FCustomSceneViewExtension::ScreenspaceAmbientOcclusionPostProcessing));
	}
}

//TODO: 拉普拉斯算子描边效果 By GYP
FScreenPassTexture FCustomSceneViewExtension::SamplePostProcessing(FRDGBuilder& GraphBuilder,
                                                                   const FSceneView& SceneView,
                                                                   const FPostProcessMaterialInputs& Inputs)
{
	const FViewInfo& View = static_cast<const FViewInfo&>(SceneView);


	const FScreenPassTexture& SceneColor = FScreenPassTexture::CopyFromSlice(
		GraphBuilder, Inputs.GetInput(EPostProcessMaterialInput::SceneColor));

	// 获取场景颜色的RDG纹理引用
	if (!SceneColor.IsValid() || CVarShaderOnSample.GetValueOnRenderThread() == 0)
	{
		return SceneColor;
	}
	RDG_EVENT_SCOPE(GraphBuilder, "Sample PostProcessing Effect");
	{
		FRDGTextureRef SceneColorTexture = SceneColor.Texture;

		// 直接使用场景颜色纹理，无需复制
		TShaderMapRef<FSamplePS> PixelShader(View.ShaderMap);
		FSamplePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FSamplePS::FParameters>();
		// Setup all the descriptors to create a target texture
		FRDGTextureDesc OutputDesc;
		{
			OutputDesc = SceneColor.Texture->Desc;

			OutputDesc.Reset();
			OutputDesc.Flags |= TexCreate_RenderTargetable | TexCreate_ShaderResource;
			//OutputDesc.Flags &= ~(TexCreate_RenderTargetable | TexCreate_FastVRAM);

			FLinearColor ClearColor(0., 0., 0., 0.);
			OutputDesc.ClearValue = FClearValueBinding(ClearColor);
		}
		FRDGTextureRef InputTexture = GraphBuilder.CreateTexture(OutputDesc, TEXT("SampleRenderPS Output Texture"));

		// 配置参数
		PassParameters->View = View.ViewUniformBuffer;
		PassParameters->SceneColorTexture = SceneColorTexture; // 使用原始场景颜色纹理
		PassParameters->SceneColorSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		PassParameters->RenderTargets[0] = FRenderTargetBinding(
			InputTexture,
			ERenderTargetLoadAction::ENoAction // 清空目标纹理
		);

		// 添加全屏Pass（输入输出尺寸需匹配）
		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("SampleRenderPS"),
			View,
			FScreenPassTextureViewport(SceneColorTexture), // 输出视口
			FScreenPassTextureViewport(SceneColorTexture), // 输入视口（自动适配尺寸）
			PixelShader,
			PassParameters
		);
		AddCopyTexturePass(GraphBuilder, InputTexture, SceneColorTexture);
	}


	return SceneColor;
}

//TODO: GYP SSR render

FScreenPassTexture FCustomSceneViewExtension::ScreenspaceReflectionPostProcessing(
	FRDGBuilder& GraphBuilder, const FSceneView& SceneView, const FPostProcessMaterialInputs& Inputs)
{
	const FViewInfo& View = static_cast<const FViewInfo&>(SceneView);

	const FScreenPassTexture& SceneColor = FScreenPassTexture::CopyFromSlice(
		GraphBuilder, Inputs.GetInput(EPostProcessMaterialInput::SceneColor));
	//CreateSceneTextureShaderParameters(GraphBuilder,SceneView,ESceneTextureSetupMode::GBuffers);

	//TODO: GYP 我改了引擎#include "SceneTextureParameters.h"文件，暴漏接口，应该可以不用修改引擎的，目前先这样吧RENDERER_API FSceneTextureParameters GetSceneTextureParameters(FRDGBuilder& GraphBuilder, const FViewInfo& View);
	const FSceneTextureParameters& SceneTextures = GetSceneTextureParameters(GraphBuilder, View);
	//const FSceneTextureShaderParameters& SceneTextures= Inputs.SceneTextures;

	// 获取场景颜色的RDG纹理引用
	if (!SceneColor.IsValid() || CVarShaderOn.GetValueOnRenderThread() == 0)
	{
		return SceneColor;
	}

	{
		RDG_EVENT_SCOPE(GraphBuilder, "GYPScreenspaceReflection");
		RDG_GPU_STAT_SCOPE(GraphBuilder, GYPScreenspaceReflection)
		FRDGTextureRef SceneColorTexture = SceneColor.Texture;

		// 直接使用场景颜色纹理，无需复制
		TShaderMapRef<FScreenspaceReflectionPS> PixelShader(View.ShaderMap);
		FScreenspaceReflectionPS::FParameters* PassParameters = GraphBuilder.AllocParameters<
			FScreenspaceReflectionPS::FParameters>();
		// Setup all the descriptors to create a target texture
		FRDGTextureDesc OutputDesc;
		{
			OutputDesc = SceneColor.Texture->Desc;

			OutputDesc.Reset();
			OutputDesc.Flags |= TexCreate_RenderTargetable | TexCreate_ShaderResource;
			//OutputDesc.Flags &= ~(TexCreate_RenderTargetable | TexCreate_FastVRAM);

			FLinearColor ClearColor(0., 0., 0., 0.);
			OutputDesc.ClearValue = FClearValueBinding(ClearColor);
		}
		FRDGTextureRef InputTexture = GraphBuilder.CreateTexture(OutputDesc, TEXT("GYPScreenSpaceReflections"));

		// 配置参数
		PassParameters->View = View.ViewUniformBuffer;
		PassParameters->SceneColorTexture = SceneColorTexture; // 使用原始场景颜色纹理
		PassParameters->SceneColorSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();


		PassParameters->HZB = View.HZB;
		PassParameters->HZBSampler = TStaticSamplerState<SF_Point>::GetRHI();
		{
			const FVector2D HZBUvFactor(
				float(View.ViewRect.Width()) / float(2 * View.HZBMipmap0Size.X),
				float(View.ViewRect.Height()) / float(2 * View.HZBMipmap0Size.Y));
			PassParameters->HZBUvFactorAndInvFactor = FVector4f(
				HZBUvFactor.X,
				HZBUvFactor.Y,
				1.0f / HZBUvFactor.X,
				1.0f / HZBUvFactor.Y);
		}
		PassParameters->SceneTextures = SceneTextures;
		PassParameters->RenderTargets[0] = FRenderTargetBinding(
			InputTexture,
			ERenderTargetLoadAction::ENoAction // 清空目标纹理
		);

		// 添加全屏Pass（输入输出尺寸需匹配）
		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("GYP ScreenspaceReflectionPS"),
			View,
			FScreenPassTextureViewport(SceneColorTexture), // 输出视口
			FScreenPassTextureViewport(SceneColorTexture), // 输入视口（自动适配尺寸）
			PixelShader,
			PassParameters
		);
		AddCopyTexturePass(GraphBuilder, InputTexture, SceneColorTexture);
	}


	return SceneColor;
}

// 实现SSAO渲染函数
FScreenPassTexture FCustomSceneViewExtension::ScreenspaceAmbientOcclusionPostProcessing(
	FRDGBuilder& GraphBuilder, const FSceneView& SceneView, const FPostProcessMaterialInputs& Inputs)
{
	const FViewInfo& View = static_cast<const FViewInfo&>(SceneView);
	const FScreenPassTexture& SceneColor = FScreenPassTexture::CopyFromSlice(
		GraphBuilder, Inputs.GetInput(EPostProcessMaterialInput::SceneColor));
	const FSceneTextureParameters& SceneTextures = GetSceneTextureParameters(GraphBuilder, View);

	if (!SceneColor.IsValid() || CVarSSAOOn.GetValueOnRenderThread() == 0)
	{
		return SceneColor;
	}
	{
		RDG_EVENT_SCOPE(GraphBuilder, "SampleSSAO");
		FRDGTextureRef SceneColorTexture = SceneColor.Texture;
		TShaderMapRef<FSampleSSAOPS> PixelShader(View.ShaderMap);
		FSampleSSAOPS::FParameters* PassParameters = GraphBuilder.AllocParameters<
			FSampleSSAOPS::FParameters>();

		FRDGTextureDesc OutputDesc;
		{
			OutputDesc = SceneColorTexture->Desc;
			OutputDesc.Reset();
			OutputDesc.Flags |= TexCreate_RenderTargetable | TexCreate_ShaderResource;
			FLinearColor ClearColor(0., 0., 0., 0.);
			OutputDesc.ClearValue = FClearValueBinding(ClearColor);
		}
		FRDGTextureRef OutputTexture = GraphBuilder.CreateTexture(OutputDesc, TEXT("SampleSSAO"));

		PassParameters->View = View.ViewUniformBuffer;
		PassParameters->SceneColorTexture = SceneColorTexture; // 使用原始场景颜色纹理
		PassParameters->SceneColorSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		PassParameters->RandomNormalTexture = GSystemTextures.SSAORandomization->GetRHI();
		PassParameters->RandomNormalSampler = TStaticSamplerState<SF_Point>::GetRHI();
		//PassParameters->TestSceneDepthTexture = SceneTextures.SceneDepthTexture; //GraphBuilder.RegisterExternalTexture(GSystemTextures.DepthDummy);
		// 确保使用正确的深度纹理
		if (SceneTextures.SceneDepthTexture)
		{
			PassParameters->TestSceneDepthTexture = SceneTextures.SceneDepthTexture;
		}
		else
		{
			// 回退方案：使用系统深度纹理
			PassParameters->TestSceneDepthTexture = GraphBuilder.RegisterExternalTexture(GSystemTextures.BlackDummy);
		}
		PassParameters->TestSceneDepthSampler = TStaticSamplerState<SF_Point>::GetRHI();
		PassParameters->HZBTexture = View.HZB;
		PassParameters->HZBSampler = TStaticSamplerState<SF_Point>::GetRHI();

		{
			const FVector2f HZBUvFactor(
				float(View.ViewRect.Width()) / float(2 * View.HZBMipmap0Size.X),
				float(View.ViewRect.Height()) / float(2 * View.HZBMipmap0Size.Y));
			PassParameters->HZBUvBiasFactor = FVector4f(
				HZBUvFactor.X,
				HZBUvFactor.Y,
				0.5f * HZBUvFactor.X,
				0.5f * HZBUvFactor.Y);
		}

		PassParameters->ScreenSpaceAOParams[0] = FVector4f(2.0f, 0.01f, 0.0f, 2.0f);  // AO强度
		PassParameters->ScreenSpaceAOParams[1] = FVector4f(2.0f, 2.0f, 20.0f, 0.0f); // 随机缩放和半径
		PassParameters->SceneTextures = SceneTextures;
		PassParameters->RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::EClear);

		// 添加全屏Pass（输入输出尺寸需匹配）
		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("SampleSSAOs"),
			View,
			FScreenPassTextureViewport(SceneColorTexture), // 输出视口
			FScreenPassTextureViewport(SceneColorTexture), // 输入视口（自动适配尺寸）
			PixelShader,
			PassParameters
		);
		AddCopyTexturePass(GraphBuilder, OutputTexture, SceneColorTexture);
	}
	return SceneColor;
}