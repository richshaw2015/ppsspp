/**
 * 鸿蒙 XComponent 集成
 * XComponent 是鸿蒙的原生渲染组件，类似 Android 的 SurfaceView
 */

#ifndef OHOS_XCOMPONENT_H
#define OHOS_XCOMPONENT_H

#include <ace/xcomponent/native_interface_xcomponent.h>
#include <string>

class OhosGLContext;
class OhosVulkanContext;
class OhosGraphicsContext;

namespace OhosXComponent {

/**
 * 图形后端类型
 */
enum class GraphicsBackend {
    OPENGL,
    VULKAN
};

/**
 * 初始化 XComponent
 * @param component XComponent 组件
 * @param backend 图形后端类型（默认 OpenGL）
 */
bool Initialize(OH_NativeXComponent* component, GraphicsBackend backend = GraphicsBackend::OPENGL);

/**
 * 关闭 XComponent
 */
void Shutdown();

/**
 * 获取当前图形后端类型
 */
GraphicsBackend GetCurrentBackend();

/**
 * 获取图形上下文（通用接口）
 */
OhosGraphicsContext* GetGraphicsContext();

/**
 * 获取 GL 上下文（兼容旧代码）
 */
OhosGLContext* GetGLContext();

/**
 * 获取 Vulkan 上下文
 */
OhosVulkanContext* GetVulkanContext();

/**
 * XComponent 回调：表面创建
 */
void OnSurfaceCreated(OH_NativeXComponent* component, void* window);

/**
 * XComponent 回调：表面改变
 */
void OnSurfaceChanged(OH_NativeXComponent* component, void* window);

/**
 * XComponent 回调：表面销毁
 */
void OnSurfaceDestroyed(OH_NativeXComponent* component, void* window);

/**
 * XComponent 回调：触摸事件
 */
void OnTouchEvent(OH_NativeXComponent* component, void* window);

/**
 * 渲染循环
 */
void RenderLoop();

/**
 * 停止渲染循环
 */
void StopRenderLoop();

/**
 * 获取当前 FPS
 */
double GetCurrentFPS();

/**
 * 获取上一帧渲染时间（秒）
 */
double GetLastFrameTime();

/**
 * 获取表面宽度
 */
int GetSurfaceWidth();

/**
 * 获取表面高度
 */
int GetSurfaceHeight();

} // namespace OhosXComponent

#endif // OHOS_XCOMPONENT_H
