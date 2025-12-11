# 缺失函数和桩实现说明

## 概述

本文档列出了 UI 库需要但平台未提供的函数，以及当前的桩实现状态。

## 更新时间
2024-12-08 (最后编译修复)

## 函数分类

### 1. 权限系统 (已实现桩)

#### System_GetPermissionStatus
```cpp
PermissionStatus System_GetPermissionStatus(SystemPermission permission);
```
**用途**: 检查权限状态  
**当前实现**: 总是返回 `PERMISSION_STATUS_GRANTED`  
**TODO**: 实现鸿蒙权限检查 API

#### System_AskForPermission
```cpp
void System_AskForPermission(SystemPermission permission);
```
**用途**: 请求权限  
**当前实现**: 空实现，记录日志  
**TODO**: 实现鸿蒙权限请求对话框

### 2. 音频后端 (已实现桩)

#### System_CreateAudioBackend
```cpp
AudioBackend *System_CreateAudioBackend();
```
**用途**: 创建音频后端  
**当前实现**: 返回 `nullptr`（使用默认实现）  
**TODO**: 实现鸿蒙音频后端（OpenSL ES 或 AudioRenderer）

### 3. VR 功能 (已实现桩)

#### UpdateVRKeys
```cpp
void UpdateVRKeys(const KeyInput &key);
```
**用途**: 更新 VR 按键输入  
**当前实现**: 空实现（鸿蒙不支持 VR）  
**TODO**: 无需实现

#### UpdateVRAxis
```cpp
void UpdateVRAxis(const AxisInput *axes, size_t count);
```
**用途**: 更新 VR 轴输入  
**当前实现**: 空实现（鸿蒙不支持 VR）  
**TODO**: 无需实现

#### SetVRAppMode
```cpp
void SetVRAppMode(VRAppMode mode);
```
**用途**: 设置 VR 应用模式  
**当前实现**: 空实现（鸿蒙不支持 VR）  
**TODO**: 无需实现

### 4. ImGui 平台层 (已实现桩)

#### ImGui_ImplPlatform_Init
```cpp
void ImGui_ImplPlatform_Init(const Path &fontPath);
```
**用途**: 初始化 ImGui 平台层  
**当前实现**: 空实现  
**TODO**: 实现鸿蒙平台的 ImGui 输入处理

#### ImGui_ImplPlatform_NewFrame
```cpp
void ImGui_ImplPlatform_NewFrame();
```
**用途**: ImGui 平台层新帧  
**当前实现**: 空实现  
**TODO**: 更新输入状态

#### ImGui_ImplPlatform_KeyEvent
```cpp
bool ImGui_ImplPlatform_KeyEvent(const KeyInput &key);
```
**用途**: 处理按键事件  
**当前实现**: 返回 `false`  
**TODO**: 转换按键事件到 ImGui

#### ImGui_ImplPlatform_TouchEvent
```cpp
bool ImGui_ImplPlatform_TouchEvent(const TouchInput &touch);
```
**用途**: 处理触摸事件  
**当前实现**: 返回 `false`  
**TODO**: 转换触摸事件到 ImGui

### 5. ImGui Thin3D 渲染层 (已实现桩)

#### ImGui_ImplThin3d_Init
```cpp
bool ImGui_ImplThin3d_Init(Draw::DrawContext *draw, 
                           const unsigned char *ttf_font, size_t ttf_size,
                           const unsigned char *ttf_font_bold, size_t ttf_size_bold);
```
**用途**: 初始化 ImGui 渲染层  
**当前实现**: 返回 `false`  
**TODO**: 实现 Thin3D 渲染后端

#### ImGui_ImplThin3d_Shutdown
```cpp
void ImGui_ImplThin3d_Shutdown();
```
**用途**: 关闭 ImGui 渲染层  
**当前实现**: 空实现  
**TODO**: 清理渲染资源

#### ImGui_ImplThin3d_NewFrame
```cpp
void ImGui_ImplThin3d_NewFrame(Draw::DrawContext *draw, Lin::Matrix4x4 drawMatrix);
```
**用途**: ImGui 渲染层新帧  
**当前实现**: 空实现  
**TODO**: 准备渲染状态

#### ImGui_ImplThin3d_CreateDeviceObjects
```cpp
bool ImGui_ImplThin3d_CreateDeviceObjects(Draw::DrawContext *draw);
```
**用途**: 创建设备对象  
**当前实现**: 返回 `false`  
**TODO**: 创建着色器、纹理等

#### ImGui_ImplThin3d_DestroyDeviceObjects
```cpp
void ImGui_ImplThin3d_DestroyDeviceObjects();
```
**用途**: 销毁设备对象  
**当前实现**: 空实现  
**TODO**: 清理设备资源

### 6. ImGui 核心函数 (已实现桩)

#### ImGui::CreateContext
```cpp
ImGuiContext* CreateContext(ImFontAtlas* shared_font_atlas);
```
**用途**: 创建 ImGui 上下文  
**当前实现**: 返回 `nullptr`  
**TODO**: 链接完整的 ImGui 库

#### ImGui::DestroyContext
```cpp
void DestroyContext(ImGuiContext* ctx);
```
**用途**: 销毁 ImGui 上下文  
**当前实现**: 空实现  
**TODO**: 链接完整的 ImGui 库

#### ImGui::GetIO
```cpp
ImGuiIO& GetIO();
```
**用途**: 获取 ImGui IO 对象  
**当前实现**: 返回静态空对象  
**TODO**: 链接完整的 ImGui 库

#### ImGui::NewFrame
```cpp
void NewFrame();
```
**用途**: 开始新的 ImGui 帧  
**当前实现**: 空实现  
**TODO**: 链接完整的 ImGui 库

#### ImGui::DebugCheckVersionAndDataLayout
```cpp
bool DebugCheckVersionAndDataLayout(...);
```
**用途**: 检查 ImGui 版本和数据布局  
**当前实现**: 返回 `true`  
**TODO**: 链接完整的 ImGui 库

## 实现优先级

### 高优先级（影响基本功能）

1. **权限系统** - 需要访问存储
   - System_GetPermissionStatus
   - System_AskForPermission

2. **音频后端** - 需要声音输出
   - System_CreateAudioBackend

### 中优先级（影响调试功能）

3. **ImGui 集成** - 调试器需要
   - 所有 ImGui 相关函数

### 低优先级（可选功能）

4. **VR 功能** - 鸿蒙不支持
   - 所有 VR 相关函数（无需实现）

## 实现建议

### 1. 权限系统

参考 HarmonyOS 权限 API：

```cpp
PermissionStatus System_GetPermissionStatus(SystemPermission permission) {
    // 使用 HarmonyOS 权限 API
    // OH_PermissionManager_CheckPermission()
    switch (permission) {
        case SYSTEM_PERMISSION_STORAGE:
            // 检查存储权限
            return PERMISSION_STATUS_GRANTED;
        default:
            return PERMISSION_STATUS_DENIED;
    }
}

void System_AskForPermission(SystemPermission permission) {
    // 使用 HarmonyOS 权限请求 API
    // OH_PermissionManager_RequestPermission()
}
```

### 2. 音频后端

创建 `ohos_audio_backend.cpp`：

```cpp
class OhosAudioBackend : public AudioBackend {
public:
    bool Init(int sampleRate, int numChannels) override;
    void Shutdown() override;
    void PushSamples(const int *audio, int numSamples) override;
    // ...
};

AudioBackend *System_CreateAudioBackend() {
    return new OhosAudioBackend();
}
```

### 3. ImGui 集成

#### 选项 A: 完整集成 ImGui 库

添加到 CMakeLists.txt：

```cmake
# ImGui 库
add_library(imgui STATIC
    ${PPSSPP_ROOT}/ext/imgui/imgui.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_draw.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_tables.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_widgets.cpp
)

target_link_libraries(UI PUBLIC imgui)
```

#### 选项 B: 禁用 ImGui 调试器

在编译时定义宏：

```cmake
add_definitions(-DPPSSPP_DISABLE_IMGUI_DEBUGGER)
```

## 当前状态

### 已实现（桩）
- ✅ 权限系统（返回固定值）
- ✅ 音频后端（返回 nullptr）
- ✅ VR 功能（空实现）
- ✅ ImGui 平台层（空实现）
- ✅ ImGui 渲染层（空实现）
- ✅ ImGui 核心函数（桩实现）
- ✅ 所有编译错误已修复（添加必要的头文件）

### 编译修复
- ✅ 添加 `Common/Math/lin/matrix4x4.h` - Lin::Matrix4x4 类型
- ✅ 添加 `Common/VR/PPSSPPVR.h` - VRAppMode 枚举
- ✅ 修复 ImGuiIO 桩实现 - 使用缓冲区避免不完整类型

### 待实现
- ⏳ 权限系统（真实实现）
- ⏳ 音频后端（真实实现）
- ⏳ ImGui 集成（可选）

## 影响分析

### 当前桩实现的影响

1. **权限系统**
   - 影响：无法正确检查权限
   - 后果：可能无法访问存储
   - 优先级：高

2. **音频后端**
   - 影响：使用默认音频实现
   - 后果：可能没有声音或性能不佳
   - 优先级：高

3. **ImGui 调试器**
   - 影响：调试器无法使用
   - 后果：无法使用内置调试工具
   - 优先级：中（开发阶段重要）

4. **VR 功能**
   - 影响：VR 功能不可用
   - 后果：无影响（鸿蒙不支持 VR）
   - 优先级：低

## 测试建议

### 基本功能测试（无需上述功能）
- ✅ UI 显示
- ✅ 游戏列表
- ✅ 设置界面
- ⚠️ 游戏加载（需要权限）
- ⚠️ 游戏运行（需要音频）

### 完整功能测试（需要实现）
- ⏳ 文件访问（需要权限系统）
- ⏳ 声音输出（需要音频后端）
- ⏳ 调试功能（需要 ImGui）

## 下一步工作

### 立即执行
1. 编译验证（桩实现足以通过链接）
2. 基本 UI 测试
3. 确认哪些功能必须实现

### 短期计划
1. 实现权限系统
2. 实现音频后端
3. 测试游戏加载和运行

### 长期计划
1. 集成 ImGui 库（如果需要调试器）
2. 优化音频性能
3. 完善权限处理

## 参考

### HarmonyOS API
- [权限管理](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/accesstoken-overview-0000001544583929-V3)
- [音频管理](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/audio-playback-0000001427584620-V3)

### PPSSPP 源码
- `UI/ImDebugger/` - ImGui 调试器实现
- `Common/System/System.h` - 系统接口定义
- `android/jni/` - Android 平台实现参考

## 总结

当前所有缺失的函数都已提供桩实现，可以通过链接。但要实现完整功能，需要：

1. **必须实现**: 权限系统、音频后端
2. **可选实现**: ImGui 集成（用于调试）
3. **无需实现**: VR 功能

建议先编译测试基本 UI，然后根据实际需求逐步实现真正的功能。
