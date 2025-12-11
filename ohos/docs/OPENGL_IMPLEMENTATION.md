# OpenGL ES 渲染管线实现

## 概述

本文档描述 PPSSPP 鸿蒙版的 OpenGL ES 渲染管线实现，参考 Android 版本的架构。

## 架构

### 1. 图形上下文层次结构

```
GraphicsContext (PPSSPP 基类)
    ↓
OhosGraphicsContext (鸿蒙适配层)
    ↓
OhosGLContext (OpenGL ES 实现)
```

参考 Android 的架构：
```
GraphicsContext
    ↓
AndroidGraphicsContext
    ↓
AndroidJavaEGLGraphicsContext
```

### 2. 核心组件

#### 2.1 OhosGLContext

**文件**: `ohos/entry/src/main/cpp/ohos_gl_context.{h,cpp}`

**职责**:
- EGL 上下文管理
- Draw::DrawContext 创建和管理
- GLRenderManager 集成
- 渲染线程生命周期管理

**关键方法**:
- `InitFromRenderThread()` - 从渲染线程初始化 EGL 和 Draw 上下文
- `ShutdownFromRenderThread()` - 从渲染线程清理资源
- `ThreadStart()` - 启动渲染线程
- `ThreadFrame()` - 处理一帧渲染
- `ThreadEnd()` - 结束渲染线程

#### 2.2 OhosXComponent

**文件**: `ohos/entry/src/main/cpp/ohos_xcomponent.{h,cpp}`

**职责**:
- XComponent 回调处理
- 表面生命周期管理
- 渲染循环控制
- 与 PPSSPP Core 集成

**XComponent 回调**:
- `OnSurfaceCreated` - 表面创建时初始化渲染
- `OnSurfaceChanged` - 表面尺寸变化时更新
- `OnSurfaceDestroyed` - 表面销毁时清理
- `DispatchTouchEvent` - 处理触摸输入

### 3. 渲染流程

#### 3.1 初始化流程

```
1. ArkTS 创建 XComponent
   ↓
2. NAPI InitXComponent() 被调用
   ↓
3. OhosXComponent::Initialize() 注册回调
   ↓
4. OnSurfaceCreated() 回调触发
   ↓
5. 启动渲染线程
   ↓
6. OhosGLContext::InitFromRenderThread()
   - 初始化 EGL
   - 创建 Draw::DrawContext
   - 获取 GLRenderManager
   - 编译预设着色器
   ↓
7. NativeInitGraphics() 初始化 PPSSPP 图形系统
   ↓
8. ThreadStart() 启动渲染管理器
```

#### 3.2 渲染循环

```
while (!exitRenderLoop) {
    1. NativeFrame(graphicsContext)
       - 更新游戏逻辑
       - 提交渲染命令
    
    2. ThreadFrame(true)
       - GLRenderManager 处理渲染队列
       - 执行 OpenGL 命令
       - 交换缓冲区
}
```

#### 3.3 清理流程

```
1. OnSurfaceDestroyed() 或应用退出
   ↓
2. StopRenderLoop() 设置退出标志
   ↓
3. 等待渲染线程结束
   ↓
4. NativeShutdownGraphics()
   ↓
5. ThreadEnd()
   ↓
6. ShutdownFromRenderThread()
   - 清理 Draw 上下文
   - 销毁 EGL 上下文
```

## 与 Android 的对比

### 相似之处

1. **架构设计**: 都使用 GraphicsContext 抽象层
2. **渲染线程**: 都在独立线程中运行渲染循环
3. **Draw 系统**: 都使用 PPSSPP 的 thin3d Draw 抽象层
4. **GLRenderManager**: 都使用相同的渲染管理器

### 差异之处

| 特性 | Android | HarmonyOS |
|------|---------|-----------|
| 表面组件 | SurfaceView/TextureView | XComponent |
| 窗口类型 | ANativeWindow | OHNativeWindow |
| 回调机制 | JNI 调用 | XComponent 回调 |
| 线程管理 | Java 线程 + JNI | C++ std::thread |

## XComponent 集成

### ArkTS 端使用

```typescript
import ppsspp from 'libppsspp_ohos.so'

@Component
struct GameView {
  private xComponentController: XComponentController = new XComponentController()
  
  build() {
    XComponent({
      id: 'ppsspp_surface',
      type: 'surface',
      controller: this.xComponentController
    })
    .onLoad(() => {
      // 初始化 XComponent
      ppsspp.initXComponent(this.xComponentController)
    })
    .width('100%')
    .height('100%')
  }
}
```

## EGL 配置

### 配置属性

```cpp
EGL_SURFACE_TYPE: EGL_WINDOW_BIT
EGL_RENDERABLE_TYPE: EGL_OPENGL_ES3_BIT
EGL_RED_SIZE: 8
EGL_GREEN_SIZE: 8
EGL_BLUE_SIZE: 8
EGL_ALPHA_SIZE: 8
EGL_DEPTH_SIZE: 24
EGL_STENCIL_SIZE: 8
```

### 上下文属性

```cpp
EGL_CONTEXT_CLIENT_VERSION: 3  // OpenGL ES 3.0
```

## 性能优化

### 1. 多缓冲

通过 `GLRenderManager::SetInflightFrames()` 配置飞行帧数：
```cpp
renderManager_->SetInflightFrames(g_Config.iInflightFrames);
```

### 2. VSync 同步

EGL 默认启用 VSync，通过 `eglSwapBuffers()` 自动同步。

### 3. 着色器预编译

在初始化时预编译常用着色器：
```cpp
draw_->CreatePresets();
```

## 调试

### 日志标签

- `PPSSPP_GL` - OpenGL 上下文相关
- `PPSSPP_XComponent` - XComponent 集成相关
- `Log::G3D` - PPSSPP 图形系统日志

### 常见问题

1. **EGL 初始化失败**
   - 检查 EGL 配置是否支持
   - 确认 native_window 有效

2. **渲染黑屏**
   - 检查 Draw 上下文是否创建成功
   - 确认着色器编译成功
   - 验证 GLRenderManager 正常工作

3. **性能问题**
   - 调整飞行帧数
   - 检查 VSync 设置
   - 分析渲染队列堆积

## 下一步

1. **Vulkan 支持** - 参考 `AndroidVulkanContext` 实现 `OhosVulkanContext`
2. **输入处理** - 完善 `OnTouchEvent` 实现
3. **音频同步** - 集成音频系统与渲染循环
4. **性能分析** - 添加帧率统计和性能监控

## 参考文件

- `android/jni/AndroidJavaGLContext.{h,cpp}` - Android OpenGL 实现
- `android/jni/app-android.cpp` - Android 应用集成
- `Common/GraphicsContext.h` - 图形上下文基类
- `Common/GPU/OpenGL/GLRenderManager.h` - OpenGL 渲染管理器
- `Common/GPU/thin3d_create.h` - Draw 上下文创建
