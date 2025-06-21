# SSPostProcessPlugin
基于Unreal Engine 5.4.4源码版实现的屏幕空间后处理插件，提供可扩展的屏幕空间渲染效果解决方案。

### 前置条件

Unreal Engine 5.4.4 源码版本

### 使用方法

#### step1 引擎修改：

PS: 应该可以不用修改引擎的，最好用SceneTextureShaderParameters获取Gbuffer，目前先这样吧...

修改源码此行暴露Gbuffer的获取接口`Engine\UE5\Source\Runtime\Renderer\Private\SceneTextureParameters.h:24`

```C++
//修改前
FSceneTextureParameters GetSceneTextureParameters(FRDGBuilder& GraphBuilder, const FViewInfo& View);
//修改后
RENDERER_API FSceneTextureParameters GetSceneTextureParameters(FRDGBuilder& GraphBuilder, const FViewInfo& View);
```

#### step2 插件安装：

将此项目clone到项目或引擎`Plugins`文件夹下，启用插件，编译运行，命令行启动，已包含功能：

| 控制台命令               | 功能描述                          | 示例值 |
| ------------------------ | --------------------------------- | ------ |
| `r.SampleSSR`            | 开/关屏幕空间反射效果，全屏幕1spp | 1/0    |
| `r.SamplePostProcessing` | 基于拉普拉斯算子的简单描边效果    | 1/0    |
| `r.SampleSSAO`           | 开/关屏幕空间环境光遮蔽效果       | 1/0    |

### TODO：

更多功能待实现
