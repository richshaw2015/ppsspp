# OHOS 平台 Vulkan 渲染引擎故障排查指南

## 问题描述

在 OHOS 平台上，设置界面的"渲染引擎"选项中只显示 OpenGL，没有 Vulkan 选项，即使设备理论上支持 Vulkan。

## 问题原因

Vulkan 选项是否显示取决于 `VulkanMayBeAvailable()` 函数的返回值。该函数会执行以下检查：

1. **设备黑名单检查** - 检查设备是否在已知不支持 Vulkan 的黑名单中
2. **Vulkan 库加载** - 尝试加载 `libvulkan.so`
3. **Vulkan 实例创建** - 创建测试用的 Vulkan 实例
4. **物理设备枚举** - 检查是否有可用的 Vulkan 物理设备
5. **GPU 黑名单检查** - 检查 GPU 是否在已知有问题的黑名单中

任何一步失败都会导致 Vulkan 被标记为不可用。

## 排查步骤

### 1. 查看应用日志

运行应用并查看 HiLog 输出，搜索以下关键字：

```bash
# 查看 Vulkan 检测日志
hdc shell hilog | grep -i vulkan

# 查看 PPSSPP 应用日志
hdc shell hilog | grep PPSSPP
```

### 2. 关键日志信息

**成功的日志示例：**
```
PPSSPP_App: === Checking Vulkan availability ===
PPSSPP_App: Device name: HUAWEI XXX
PPSSPP_App: System version: 5
G3D: VulkanMayBeAvailable: Device allowed ('HUAWEI XXX')
G3D: Vulkan library loaded ('libvulkan.so')
G3D: VulkanMayBeAvailable: Vulkan test instance created successfully.
G3D: VulkanMayBeAvailable: Eligible device found: 'Mali-G78'
G3D: VulkanMayBeAvailable: Found working Vulkan API!
PPSSPP_App: Vulkan available: YES
```

**失败的日志示例：**

#### 情况 1：库未找到
```
G3D: Vulkan loader: Library not available: No vulkan library found
PPSSPP_App: Vulkan available: NO
PPSSPP_App: Possible reasons:
PPSSPP_App:   1. libvulkan.so not found in system
```

#### 情况 2：实例创建失败
```
G3D: Vulkan library loaded ('libvulkan.so')
G3D: VulkanMayBeAvailable: Failed to create vulkan instance (VK_ERROR_INCOMPATIBLE_DRIVER)
PPSSPP_App: Vulkan available: NO
```

#### 情况 3：无可用设备
```
G3D: VulkanMayBeAvailable: Vulkan test instance created successfully.
G3D: VulkanMayBeAvailable: No physical Vulkan devices (count = 0).
PPSSPP_App: Vulkan available: NO
```

### 3. 检查系统 Vulkan 支持

在设备上执行以下命令检查 Vulkan 库：

```bash
# 检查 Vulkan 库是否存在
hdc shell ls -l /system/lib64/libvulkan.so
hdc shell ls -l /system/lib/libvulkan.so

# 检查 Vulkan 版本信息（如果有 vulkaninfo 工具）
hdc shell vulkaninfo
```

### 4. 验证设备 Vulkan 能力

OHOS 设备的 Vulkan 支持取决于：

1. **硬件支持** - GPU 必须支持 Vulkan
2. **驱动支持** - GPU 驱动必须实现 Vulkan API
3. **系统版本** - HarmonyOS NEXT (API 12+) 应该支持 Vulkan

常见支持 Vulkan 的 GPU：
- Mali-G78, Mali-G710 等（ARM）
- Adreno 6xx, 7xx 系列（Qualcomm）
- PowerVR 等

## 解决方案

### 方案 1：等待系统更新

如果设备硬件支持 Vulkan 但系统未提供，需要等待 HarmonyOS 系统更新。

### 方案 2：使用 OpenGL 渲染

OpenGL ES 3.x 在 OHOS 上已经完全支持，性能也很好。如果 Vulkan 不可用，使用 OpenGL 是最佳选择。

### 方案 3：强制启用 Vulkan（仅用于调试）

**警告：** 仅在确认设备支持 Vulkan 但检测失败时使用。

修改 `ohos/entry/src/main/cpp/ohos_app.cpp`：

```cpp
// 在 Initialize() 函数中，Vulkan 检测之后添加：
if (!vulkanAvailable) {
    OHOS_LOGW(APP_TAG, "Force enabling Vulkan for testing");
    VulkanSetAvailable(true);
}
```

### 方案 4：添加设备白名单

如果你的设备型号被错误地加入黑名单，可以修改 `Common/GPU/Vulkan/VulkanLoader.cpp`：

```cpp
static const char * const g_deviceNameBlacklist[] = {
    // 移除你的设备型号
};
```

## 代码位置

相关代码文件：

1. **Vulkan 检测逻辑**
   - `Common/GPU/Vulkan/VulkanLoader.cpp` - `VulkanMayBeAvailable()`
   
2. **渲染引擎选项**
   - `UI/GameSettingsScreen.cpp` - `CreateGraphicsSettings()`
   - `Core/Config.cpp` - `IsBackendEnabled()`

3. **OHOS 初始化**
   - `ohos/entry/src/main/cpp/ohos_app.cpp` - `Initialize()`

4. **Vulkan 库路径配置**
   - `Common/GPU/Vulkan/VulkanLoader.cpp` - `so_names[]` 数组

## 常见问题

### Q: 为什么 Android 版本有 Vulkan 但 OHOS 版本没有？

A: 可能的原因：
1. OHOS 系统版本较低，Vulkan 支持不完整
2. OHOS 的 Vulkan 驱动实现与 Android 不同
3. 设备厂商在 OHOS 版本中未启用 Vulkan

### Q: 如何确认我的设备是否真的支持 Vulkan？

A: 
1. 查看设备规格，确认 GPU 型号
2. 检查系统是否有 `/system/lib64/libvulkan.so`
3. 查看应用日志中的 Vulkan 检测详情

### Q: OpenGL 和 Vulkan 性能差异大吗？

A: 
- 对于 PSP 模拟，两者性能差异通常不大
- Vulkan 在某些场景下可能更高效（多线程渲染）
- OpenGL 更成熟稳定，兼容性更好

## 技术细节

### Vulkan 库搜索路径（OHOS）

```cpp
static const char * const so_names[] = {
#elif PPSSPP_PLATFORM(OHOS)
    "libvulkan.so",                    // 标准路径
    "/system/lib64/libvulkan.so",      // 64位系统库
    "/system/lib/libvulkan.so",        // 32位系统库
#endif
};
```

### 平台 Surface 扩展

OHOS 使用 Android Surface 扩展：
```cpp
#elif defined(__ANDROID__) || defined(OHOS) || defined(__OHOS__)
    const char *platformSurfaceExtension = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#endif
```

## 更新日志

- 2024-12-12: 创建文档，添加详细的排查步骤和解决方案
