# OHOS Vulkan 渲染实现

## 概述

OHOS 平台的 Vulkan 渲染实现参考了 Android 的实现（`AndroidVulkanContext`），利用 OHOS 与 Android 在 Vulkan 层面的兼容性。

## 架构

```
┌─────────────────────────────────────────────────────────────┐
│                    OhosXComponent                            │
│  ┌─────────────────────────────────────────────────────────┐│
│  │              GraphicsBackend 选择                        ││
│  │         OPENGL ──────────┬────────── VULKAN             ││
│  └─────────────────────────────────────────────────────────┘│
│              │                              │                │
│              ▼                              ▼                │
│  ┌─────────────────────┐      ┌─────────────────────────┐   │
│  │   OhosGLContext     │      │   OhosVulkanContext     │   │
│  │   (OpenGL ES 3.0)   │      │   (Vulkan 1.0+)         │   │
│  └─────────────────────┘      └─────────────────────────┘   │
│              │                              │                │
│              ▼                              ▼                │
│  ┌─────────────────────┐      ┌─────────────────────────┐   │
│  │   GLRenderManager   │      │   VulkanRenderManager   │   │
│  └─────────────────────┘      └─────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## 关键文件

### 新增文件
- `ohos_vulkan_context.h` - Vulkan 上下文头文件
- `ohos_vulkan_context.cpp` - Vulkan 上下文实现

### 修改的文件
- `ohos_xcomponent.h` - 添加 GraphicsBackend 枚举和 Vulkan 相关接口
- `ohos_xcomponent.cpp` - 添加 Vulkan 后端支持
- `CMakeLists.txt` - 添加 Vulkan 编译选项和链接

### PPSSPP 核心修改
- `Common/GPU/Vulkan/VulkanLoader.h` - 添加 OHOS 平台支持
- `Common/GPU/Vulkan/VulkanLoader.cpp` - 添加 OHOS 平台支持
- `Common/GPU/Vulkan/VulkanContext.h` - 添加 OHOS WindowSystem
- `Common/GPU/Vulkan/VulkanContext.cpp` - 添加 OHOS surface 创建

## 实现细节

### 1. 平台兼容性

OHOS 使用与 Android 相同的 Vulkan 扩展：
- `VK_KHR_ANDROID_SURFACE_EXTENSION_NAME`
- `VK_KHR_SURFACE_EXTENSION_NAME`

OHOS 的 `OHNativeWindow` 与 Android 的 `ANativeWindow` 兼容，可以直接用于创建 Vulkan surface。

### 2. 初始化流程

```cpp
// 1. 主线程：初始化 Vulkan API
OhosVulkanContext::InitAPI()
    ├── init_glslang()
    ├── VulkanLoad()
    └── VulkanContext::CreateInstanceAndDevice()

// 2. 渲染线程：初始化 Surface 和 Swapchain
OhosVulkanContext::InitFromRenderThread()
    ├── VulkanContext::InitSurface(WINDOWSYSTEM_ANDROID, window)
    ├── Draw::T3DCreateVulkanContext()
    ├── VulkanContext::InitSwapchain()
    └── VulkanRenderManager::SetInflightFrames()
```

### 3. 渲染循环

```cpp
while (!exitLoop) {
    NativeFrame(ctx);  // 渲染一帧
    // Vulkan 不需要手动调用 ThreadFrame
    // Swapchain 的 present 由 VulkanRenderManager 处理
}
```

### 4. 后端选择

在 `OhosXComponent::Initialize()` 中可以选择图形后端：

```cpp
// 使用 OpenGL（默认）
OhosXComponent::Initialize(component, GraphicsBackend::OPENGL);

// 使用 Vulkan
OhosXComponent::Initialize(component, GraphicsBackend::VULKAN);
```

## 编译配置

### CMakeLists.txt

```cmake
# 启用 Vulkan 支持
option(USE_VULKAN "Enable Vulkan rendering backend" ON)

# Vulkan 宏定义
if(USE_VULKAN)
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
endif()

# Vulkan 源文件
if(USE_VULKAN)
    list(APPEND OHOS_SOURCES ohos_vulkan_context.cpp)
endif()

# 链接 Vulkan 库
if(USE_VULKAN)
    target_link_libraries(ppsspp_ohos PUBLIC vulkan)
endif()
```

## 使用方法

### 在 ArkTS 中选择后端

```typescript
// 可以通过配置或用户选择来决定使用哪个后端
// 然后在初始化时传递给 native 层
```

### 在 C++ 中切换后端

```cpp
// 在 napi_init.cpp 或其他初始化代码中
GraphicsBackend backend = GraphicsBackend::VULKAN;  // 或 OPENGL
OhosXComponent::Initialize(component, backend);
```

## 注意事项

1. **Vulkan 可用性检查**：在初始化 Vulkan 之前，应该检查设备是否支持 Vulkan。如果不支持，会自动回退到 OpenGL。

2. **线程安全**：Vulkan API 的初始化（`InitAPI`）在主线程进行，Surface 和 Swapchain 的创建（`InitFromRenderThread`）在渲染线程进行。

3. **资源清理**：确保在 `ShutdownFromRenderThread` 中正确清理 Vulkan 资源，避免内存泄漏。

4. **Present Mode**：默认使用 `VK_PRESENT_MODE_FIFO_KHR`（垂直同步），可以根据配置选择其他模式。

## 与 Android 实现的差异

| 方面 | Android | OHOS |
|------|---------|------|
| Surface 创建 | `vkCreateAndroidSurfaceKHR` | 相同 |
| Window 类型 | `ANativeWindow` | `OHNativeWindow`（兼容） |
| 库加载 | `libvulkan.so` | 相同 |
| 后端选择 | Java 层配置 | ArkTS 层配置 |

## 测试

1. 确保设备支持 Vulkan
2. 在配置中选择 Vulkan 后端
3. 启动应用，检查日志确认使用 Vulkan
4. 运行游戏，验证渲染正确性
