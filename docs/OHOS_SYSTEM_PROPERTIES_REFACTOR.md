# OHOS 系统属性重构总结

## 📋 重构概述

参考 Android 的实现方式，将 OHOS 平台的系统属性从**硬编码**改为**动态获取**。

## 🔄 主要改动

### 1. 引入全局变量（参考 Android）

**之前**：每次调用 `System_GetProperty()` 都返回硬编码的字符串
```cpp
case SYSPROP_NAME:
    return "HarmonyOS";  // 硬编码
```

**现在**：使用全局变量存储，支持动态更新
```cpp
static std::string systemName = "HarmonyOS";  // 默认值

case SYSPROP_NAME:
    return systemName;  // 返回动态值
```

### 2. 新增初始化函数

```cpp
// 从 ArkTS 层调用，设置真实的设备信息
void OhosSystemProperties_Init(
    const char* deviceName,      // 设备名称
    const char* deviceBuild,     // 系统版本
    const char* language,        // 语言区域
    int osVersion,               // 系统版本号
    int devType,                 // 设备类型
    int xres, int yres,          // 屏幕分辨率
    int dpi,                     // 屏幕 DPI
    float refreshRate            // 刷新率
);

void OhosSystemProperties_SetSafeInsets(float left, float top, float right, float bottom);
void OhosSystemProperties_SetAudioConfig(int rate, int frames, int optRate, int optFrames);
```

### 3. 分离属性获取函数（参考 Android）

**之前**：所有属性都在 `System_GetProperty()` 中处理，返回字符串
```cpp
std::string System_GetProperty(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_DISPLAY_XRES:
            return "1080";  // 字符串
        // ...
    }
}
```

**现在**：按类型分离，提高效率
```cpp
// 字符串属性
std::string System_GetProperty(SystemProperty prop);

// 整数属性
int64_t System_GetPropertyInt(SystemProperty prop);

// 浮点属性
float System_GetPropertyFloat(SystemProperty prop);

// 布尔属性
bool System_GetPropertyBool(SystemProperty prop);

// 字符串数组属性
std::vector<std::string> System_GetPropertyStringVec(SystemProperty prop);
```

## 📊 属性分类

### 字符串属性（System_GetProperty）
- `SYSPROP_NAME` - 设备名称
- `SYSPROP_SYSTEMBUILD` - 系统版本
- `SYSPROP_LANGREGION` - 语言区域
- `SYSPROP_CPUINFO` - CPU 信息
- `SYSPROP_BOARDNAME` - 主板名称
- `SYSPROP_GPUDRIVER_VERSION` - GPU 驱动版本
- `SYSPROP_BUILD_VERSION` - PPSSPP 版本
- `SYSPROP_TEMP_DIRS` - 临时目录

### 整数属性（System_GetPropertyInt）
- `SYSPROP_SYSTEMVERSION` - 系统版本号
- `SYSPROP_DEVICE_TYPE` - 设备类型
- `SYSPROP_DISPLAY_XRES` - 屏幕宽度
- `SYSPROP_DISPLAY_YRES` - 屏幕高度
- `SYSPROP_DISPLAY_DPI` - 屏幕 DPI
- `SYSPROP_DISPLAY_LOGICAL_DPI` - 逻辑 DPI
- `SYSPROP_DISPLAY_COUNT` - 显示器数量
- `SYSPROP_AUDIO_SAMPLE_RATE` - 音频采样率
- `SYSPROP_AUDIO_FRAMES_PER_BUFFER` - 音频缓冲区帧数
- `SYSPROP_AUDIO_OPTIMAL_*` - 最佳音频配置

### 浮点属性（System_GetPropertyFloat）
- `SYSPROP_DISPLAY_REFRESH_RATE` - 刷新率
- `SYSPROP_DISPLAY_SAFE_INSET_*` - 安全区域（刘海屏）

### 布尔属性（System_GetPropertyBool）
- `SYSPROP_HAS_FILE_BROWSER` - 支持文件浏览器
- `SYSPROP_HAS_KEYBOARD` - 支持键盘
- `SYSPROP_KEYBOARD_IS_SOFT` - 软键盘
- `SYSPROP_HAS_TEXT_CLIPBOARD` - 支持剪贴板
- 等等...

### 字符串数组属性（System_GetPropertyStringVec）
- `SYSPROP_ADDITIONAL_STORAGE_DIRS` - 额外存储目录

## 🎯 使用流程

### 1. C++ 层（已完成）
```cpp
// ohos_system_properties.cpp
static std::string systemName = "HarmonyOS";  // 默认值
static int display_xres = 1080;

void OhosSystemProperties_Init(...) {
    systemName = deviceName;
    display_xres = xres;
    // ...
}
```

### 2. NAPI 层（需要实现）
```cpp
// napi_ppsspp.cpp
static napi_value InitSystemProperties(napi_env env, napi_callback_info info) {
    // 获取 ArkTS 传入的参数
    // 调用 OhosSystemProperties_Init()
}
```

### 3. ArkTS 层（需要实现）
```typescript
// EntryAbility.ets
onCreate() {
    // 获取设备信息
    const deviceName = deviceInfo.productModel;
    const xres = display.getDefaultDisplaySync().width;
    // ...
    
    // 调用 NAPI
    ppsspp.initSystemProperties(deviceName, ..., xres, ...);
}
```

## ✅ 优势

### 1. 准确性
- ✅ 真实的设备名称和型号
- ✅ 真实的屏幕分辨率和 DPI
- ✅ 真实的系统版本和语言

### 2. 灵活性
- ✅ 支持不同设备（手机、平板、电视）
- ✅ 支持不同屏幕尺寸
- ✅ 支持刘海屏等特殊屏幕

### 3. 可维护性
- ✅ 与 Android 实现一致
- ✅ 代码结构清晰
- ✅ 易于扩展

### 4. 性能
- ✅ 按类型分离，避免字符串转换
- ✅ 直接返回原始类型
- ✅ 减少不必要的计算

## 📝 待办事项

### 高优先级
1. ⬜ 在 `napi_ppsspp.cpp` 中添加 NAPI 绑定
2. ⬜ 在 `EntryAbility.ets` 中调用初始化函数
3. ⬜ 在 `Index.ets` 中设置安全区域
4. ⬜ 测试不同设备上的属性值

### 中优先级
1. ⬜ 实现音频配置的动态获取
2. ⬜ 实现 CPU 信息的动态获取
3. ⬜ 实现 GPU 驱动版本的动态获取

### 低优先级
1. ⬜ 支持多显示器
2. ⬜ 支持额外存储目录
3. ⬜ 支持剪贴板操作

## 🔍 对比：Android vs OHOS

### Android 实现
```cpp
// app-android.cpp
static std::string systemName;
static int display_xres;

// 从 JNI 调用设置
Java_org_ppsspp_ppsspp_NativeApp_init(JNIEnv *env, ...) {
    systemName = GetJavaString(env, ...);
    display_xres = ...;
}

std::string System_GetProperty(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_NAME:
            return systemName;
    }
}

int64_t System_GetPropertyInt(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_DISPLAY_XRES:
            return display_xres;
    }
}
```

### OHOS 实现（新）
```cpp
// ohos_system_properties.cpp
static std::string systemName;
static int display_xres;

// 从 NAPI 调用设置
void OhosSystemProperties_Init(...) {
    systemName = deviceName;
    display_xres = xres;
}

std::string System_GetProperty(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_NAME:
            return systemName;
    }
}

int64_t System_GetPropertyInt(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_DISPLAY_XRES:
            return display_xres;
    }
}
```

**结论**：结构完全一致，只是调用方式从 JNI 改为 NAPI。

## 📚 相关文档

- [OHOS_SYSTEM_PROPERTIES_INIT.md](./OHOS_SYSTEM_PROPERTIES_INIT.md) - 详细的初始化指南
- [OHOS_ADAPTATION_COMPLETED.md](./OHOS_ADAPTATION_COMPLETED.md) - 平台适配完成报告
- [OHOS_PLATFORM_ADAPTATION_CHECKLIST.md](./OHOS_PLATFORM_ADAPTATION_CHECKLIST.md) - 适配检查清单

## 🎉 总结

本次重构将 OHOS 系统属性的实现从**硬编码**改为**动态获取**，完全参考了 Android 的成熟实现。这样可以：

1. ✅ 获取真实的设备信息
2. ✅ 支持不同的设备和屏幕
3. ✅ 提高代码的可维护性
4. ✅ 与 Android 实现保持一致

下一步需要在 ArkTS 层实现初始化调用，将真实的设备信息传递给 C++ 层。
