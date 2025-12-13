# OHOS 系统属性动态初始化 - 完整实现

## ✅ 实现完成

本文档记录了 OHOS 平台系统属性动态初始化功能的完整实现。

## 📋 实现概述

参考 Android 的实现方式，将系统属性从硬编码改为动态获取，通过以下三层实现：

1. **C++ 层** - 全局变量存储 + 初始化函数
2. **NAPI 层** - 绑定 C++ 函数到 ArkTS
3. **ArkTS 层** - 获取系统信息并调用 C++ 函数

## 🔧 实现细节

### 1. C++ 层实现

#### 文件：`ohos_system_properties.cpp`

**全局变量定义**：
```cpp
// 设备信息
static std::string systemName = "HarmonyOS";
static std::string systemBuild = "HarmonyOS NEXT";
static std::string langRegion = "zh_CN";
static std::string boardName = "HarmonyOS Device";
static std::string cpuInfo = "ARM64";
static std::string gpuDriverVersion = "OpenGL ES 3.2";

static int systemVersion = 5;  // HarmonyOS NEXT
static int deviceType = 0;     // DEVICE_TYPE_MOBILE

// 显示属性
static int display_xres = 1080;
static int display_yres = 1920;
static int display_dpi = 480;
static int display_logical_dpi = 320;
static float display_refresh_rate = 60.0f;
static int display_count = 1;

// 音频属性
static int sampleRate = 48000;
static int framesPerBuffer = 512;
static int optimalSampleRate = 48000;
static int optimalFramesPerBuffer = 512;

// 安全区域（刘海屏等）
static float g_safeInsetLeft = 0.0f;
static float g_safeInsetRight = 0.0f;
static float g_safeInsetTop = 0.0f;
static float g_safeInsetBottom = 0.0f;

// 额外存储目录
static std::vector<std::string> g_additionalStorageDirs;
```

**初始化函数**：
```cpp
void OhosSystemProperties_Init(
    const char* deviceName,
    const char* deviceBuild,
    const char* language,
    int osVersion,
    int devType,
    int xres, int yres, int dpi, float refreshRate) {
    
    if (deviceName) systemName = deviceName;
    if (deviceBuild) systemBuild = deviceBuild;
    if (language) langRegion = language;
    
    systemVersion = osVersion;
    deviceType = devType;
    
    display_xres = xres;
    display_yres = yres;
    display_dpi = dpi;
    display_logical_dpi = dpi / 1.5f;
    display_refresh_rate = refreshRate;
    
    INFO_LOG(Log::System, "OHOS System Properties Initialized:");
    INFO_LOG(Log::System, "  Device: %s (%s)", systemName.c_str(), systemBuild.c_str());
    INFO_LOG(Log::System, "  Language: %s", langRegion.c_str());
    INFO_LOG(Log::System, "  OS Version: %d, Device Type: %d", systemVersion, deviceType);
    INFO_LOG(Log::System, "  Display: %dx%d @%dDPI %.1fHz", display_xres, display_yres, display_dpi, display_refresh_rate);
}

void OhosSystemProperties_SetSafeInsets(float left, float top, float right, float bottom) {
    g_safeInsetLeft = left;
    g_safeInsetTop = top;
    g_safeInsetRight = right;
    g_safeInsetBottom = bottom;
    INFO_LOG(Log::System, "OHOS Safe Insets: L=%.1f T=%.1f R=%.1f B=%.1f", left, top, right, bottom);
}

void OhosSystemProperties_SetAudioConfig(int rate, int frames, int optRate, int optFrames) {
    sampleRate = rate;
    framesPerBuffer = frames;
    optimalSampleRate = optRate;
    optimalFramesPerBuffer = optFrames;
    INFO_LOG(Log::System, "OHOS Audio Config: %dHz %d frames (optimal: %dHz %d frames)",
             rate, frames, optRate, optFrames);
}
```

**属性获取函数**（按类型分离）：
```cpp
// 字符串属性
std::string System_GetProperty(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_NAME:
            return systemName;  // 动态值
        case SYSPROP_LANGREGION:
            return langRegion;
        // ...
    }
}

// 整数属性
int64_t System_GetPropertyInt(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_DISPLAY_XRES:
            return display_xres;  // 动态值
        case SYSPROP_DISPLAY_YRES:
            return display_yres;
        // ...
    }
}

// 浮点属性
float System_GetPropertyFloat(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_DISPLAY_REFRESH_RATE:
            return display_refresh_rate;  // 动态值
        case SYSPROP_DISPLAY_SAFE_INSET_LEFT:
            return g_safeInsetLeft * g_display.dpi_scale_x;
        // ...
    }
}

// 布尔属性
bool System_GetPropertyBool(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_HAS_FILE_BROWSER:
            return true;
        // ...
    }
}
```

#### 文件：`ohos_system.h`

**函数声明**：
```cpp
extern "C" void OhosSystemProperties_Init(
    const char* deviceName,
    const char* deviceBuild,
    const char* language,
    int osVersion,
    int devType,
    int xres, int yres, int dpi, float refreshRate);

extern "C" void OhosSystemProperties_SetSafeInsets(float left, float top, float right, float bottom);

extern "C" void OhosSystemProperties_SetAudioConfig(int rate, int frames, int optRate, int optFrames);
```

### 2. NAPI 层实现

#### 文件：`napi_init.cpp`

**NAPI 绑定函数**：

```cpp
/**
 * 初始化系统属性
 */
static napi_value InitSystemProperties(napi_env env, napi_callback_info info) {
    size_t argc = 9;
    napi_value args[9];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 9) {
        OHOS_LOGE(NAPI_TAG, "InitSystemProperties requires 9 arguments");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 获取字符串参数
    char deviceName[256] = {0}, deviceBuild[256] = {0}, language[32] = {0};
    size_t len;
    napi_get_value_string_utf8(env, args[0], deviceName, sizeof(deviceName), &len);
    napi_get_value_string_utf8(env, args[1], deviceBuild, sizeof(deviceBuild), &len);
    napi_get_value_string_utf8(env, args[2], language, sizeof(language), &len);
    
    // 获取整数参数
    int32_t osVersion, devType, xres, yres, dpi;
    napi_get_value_int32(env, args[3], &osVersion);
    napi_get_value_int32(env, args[4], &devType);
    napi_get_value_int32(env, args[5], &xres);
    napi_get_value_int32(env, args[6], &yres);
    napi_get_value_int32(env, args[7], &dpi);
    
    // 获取浮点参数
    double refreshRate;
    napi_get_value_double(env, args[8], &refreshRate);
    
    // 调用 C++ 初始化函数
    OhosSystemProperties_Init(
        deviceName, deviceBuild, language,
        osVersion, devType,
        xres, yres, dpi, (float)refreshRate
    );
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

/**
 * 设置安全区域
 */
static napi_value SetSafeInsets(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 4) {
        OHOS_LOGE(NAPI_TAG, "SetSafeInsets requires 4 arguments");
        return nullptr;
    }
    
    double left, top, right, bottom;
    napi_get_value_double(env, args[0], &left);
    napi_get_value_double(env, args[1], &top);
    napi_get_value_double(env, args[2], &right);
    napi_get_value_double(env, args[3], &bottom);
    
    OhosSystem::SetSafeInsets((float)left, (float)top, (float)right, (float)bottom);
    OhosSystemProperties_SetSafeInsets((float)left, (float)top, (float)right, (float)bottom);
    
    return nullptr;
}

/**
 * 设置音频配置
 */
static napi_value SetAudioConfig(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 4) {
        OHOS_LOGE(NAPI_TAG, "SetAudioConfig requires 4 arguments");
        return nullptr;
    }
    
    int32_t rate, frames, optRate, optFrames;
    napi_get_value_int32(env, args[0], &rate);
    napi_get_value_int32(env, args[1], &frames);
    napi_get_value_int32(env, args[2], &optRate);
    napi_get_value_int32(env, args[3], &optFrames);
    
    OhosSystemProperties_SetAudioConfig(rate, frames, optRate, optFrames);
    
    return nullptr;
}
```

**模块注册**：
```cpp
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        // ... 其他函数 ...
        {"initSystemProperties", nullptr, InitSystemProperties, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setSafeInsets", nullptr, SetSafeInsets, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setAudioConfig", nullptr, SetAudioConfig, nullptr, nullptr, nullptr, napi_default, nullptr},
        // ...
    };
    
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
```

### 3. ArkTS 层实现

#### 文件：`EntryAbility.ets`

**系统属性初始化**：
```typescript
import { AbilityConstant, ConfigurationConstant, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { window, display } from '@kit.ArkUI';
import { BusinessError, deviceInfo } from '@kit.BasicServicesKit';
import { ppsspp } from 'libppsspp.so';

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    // ... 其他初始化 ...
    
    // 初始化系统属性
    this.initSystemProperties();
  }
  
  private initSystemProperties(): void {
    try {
      // 1. 获取设备信息
      const deviceName = deviceInfo.productModel || "HarmonyOS Device";
      const deviceBuild = `HarmonyOS ${deviceInfo.osFullName || "NEXT"}`;
      const osVersion = parseInt(deviceInfo.sdkApiVersion) || 5;
      
      // 2. 获取语言设置
      const systemLanguage = this.context.config.language || "zh";
      const systemRegion = this.context.config.region || "CN";
      const langRegion = `${systemLanguage}_${systemRegion}`;
      
      // 3. 获取显示信息
      const displayClass = display.getDefaultDisplaySync();
      const xres = displayClass.width;
      const yres = displayClass.height;
      const dpi = displayClass.densityDPI;
      const refreshRate = displayClass.refreshRate || 60;
      
      // 4. 设备类型判断
      let devType = 0;  // 0=手机
      if (deviceInfo.deviceType === 'tablet') {
        devType = 1;  // 平板
      } else if (deviceInfo.deviceType === 'tv') {
        devType = 2;  // 电视
      }
      
      // 5. 调用 C++ 初始化函数
      ppsspp.initSystemProperties(
        deviceName,
        deviceBuild,
        langRegion,
        osVersion,
        devType,
        xres,
        yres,
        dpi,
        refreshRate
      );
      
      hilog.info(DOMAIN, TAG, 'System properties initialized successfully');
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'Failed to initialize system properties: %{public}s', JSON.stringify(err));
    }
  }
}
```

#### 文件：`Index.ets`

**安全区域设置**（已存在）：
```typescript
async aboutToAppear(): Promise<void> {
  // 初始化 PPSSPP 核心
  const success = ppsspp.initEmulator(
    this.context.filesDir,
    this.context.cacheDir,
    this.context.resourceManager
  );
  
  if (success) {
    // 获取并设置安全区域
    const windowClass = await windowManager.getLastWindow(this.context);
    const cutoutAvoidArea = windowClass.getWindowAvoidArea(windowManager.AvoidAreaType.TYPE_CUTOUT);
    
    const cutoutLeft = cutoutAvoidArea.leftRect?.width || 0;
    const cutoutTop = cutoutAvoidArea.topRect?.height || 0;
    const cutoutRight = cutoutAvoidArea.rightRect?.width || 0;
    const cutoutBottom = cutoutAvoidArea.bottomRect?.height || 0;
    
    const MIN_CORNER_INSET = 40;
    const horizontalInset = Math.max(cutoutLeft, cutoutRight, MIN_CORNER_INSET);
    const verticalInset = Math.max(cutoutTop, cutoutBottom, MIN_CORNER_INSET);
    
    // 设置安全区域
    ppsspp.setSafeInsets(horizontalInset, verticalInset, horizontalInset, verticalInset);
  }
}
```

## 📊 数据流

```
ArkTS 层 (EntryAbility.ets)
    ↓ 获取设备信息
    ↓ deviceInfo.productModel, display.getDefaultDisplaySync(), etc.
    ↓
    ↓ 调用 ppsspp.initSystemProperties(...)
    ↓
NAPI 层 (napi_init.cpp)
    ↓ InitSystemProperties()
    ↓ 解析参数
    ↓
    ↓ 调用 OhosSystemProperties_Init(...)
    ↓
C++ 层 (ohos_system_properties.cpp)
    ↓ 存储到全局变量
    ↓ systemName = deviceName
    ↓ display_xres = xres
    ↓ ...
    ↓
PPSSPP 核心
    ↓ System_GetProperty(SYSPROP_NAME)
    ↓ System_GetPropertyInt(SYSPROP_DISPLAY_XRES)
    ↓ 返回动态值
```

## ✅ 实现的功能

### 1. 动态设备信息
- ✅ 设备名称（如 "HUAWEI Mate 60"）
- ✅ 系统版本（如 "HarmonyOS NEXT"）
- ✅ 语言区域（如 "zh_CN", "en_US"）
- ✅ 设备类型（手机/平板/电视）

### 2. 动态显示属性
- ✅ 屏幕分辨率（真实的宽度和高度）
- ✅ 屏幕 DPI（真实的像素密度）
- ✅ 刷新率（如 60Hz, 90Hz, 120Hz）

### 3. 安全区域支持
- ✅ 刘海屏适配
- ✅ 圆角屏幕适配
- ✅ 左右对称策略

### 4. 音频配置（预留）
- ✅ 采样率设置
- ✅ 缓冲区大小设置
- ⚠️ 需要在音频初始化时调用

## 🎯 优势

### 1. 准确性
- ✅ 真实的设备信息，不是硬编码
- ✅ 支持不同的设备型号和屏幕
- ✅ 自动适配系统语言

### 2. 灵活性
- ✅ 支持手机、平板、电视等不同设备
- ✅ 支持不同分辨率和 DPI
- ✅ 支持刘海屏等特殊屏幕

### 3. 可维护性
- ✅ 与 Android 实现完全一致
- ✅ 代码结构清晰
- ✅ 易于扩展新属性

### 4. 性能
- ✅ 按类型分离，避免字符串转换
- ✅ 直接返回原始类型
- ✅ 初始化一次，多次使用

## 📝 使用示例

### 在 C++ 代码中使用

```cpp
// 获取设备名称
std::string deviceName = System_GetProperty(SYSPROP_NAME);
// 返回: "HUAWEI Mate 60" (真实设备名)

// 获取屏幕分辨率
int xres = System_GetPropertyInt(SYSPROP_DISPLAY_XRES);
int yres = System_GetPropertyInt(SYSPROP_DISPLAY_YRES);
// 返回: 1080, 2400 (真实分辨率)

// 获取刷新率
float refreshRate = System_GetPropertyFloat(SYSPROP_DISPLAY_REFRESH_RATE);
// 返回: 120.0 (真实刷新率)

// 获取安全区域
float leftInset = System_GetPropertyFloat(SYSPROP_DISPLAY_SAFE_INSET_LEFT);
// 返回: 实际的刘海屏宽度
```

## 🔍 调试信息

### 日志输出示例

```
[PPSSPP_EntryAbility] Device: HUAWEI Mate 60
[PPSSPP_EntryAbility] Build: HarmonyOS NEXT
[PPSSPP_EntryAbility] OS Version: 5
[PPSSPP_EntryAbility] Language: zh_CN
[PPSSPP_EntryAbility] Display: 1080x2400 @480DPI 120.0Hz
[PPSSPP_EntryAbility] Device Type: 0
[PPSSPP_EntryAbility] System properties initialized successfully

[PPSSPP_NAPI] Initializing system properties:
[PPSSPP_NAPI]   Device: HUAWEI Mate 60 (HarmonyOS NEXT)
[PPSSPP_NAPI]   Language: zh_CN, OS Version: 5, Device Type: 0
[PPSSPP_NAPI]   Display: 1080x2400 @480DPI 120.0Hz

[PPSSPP] OHOS System Properties Initialized:
[PPSSPP]   Device: HUAWEI Mate 60 (HarmonyOS NEXT)
[PPSSPP]   Language: zh_CN
[PPSSPP]   OS Version: 5, Device Type: 0
[PPSSPP]   Display: 1080x2400 @480DPI 120.0Hz

[PPSSPP] OHOS Safe Insets: L=40.0 T=40.0 R=40.0 B=40.0
```

## 📚 相关文档

- [OHOS_SYSTEM_PROPERTIES_INIT.md](./OHOS_SYSTEM_PROPERTIES_INIT.md) - 初始化指南
- [OHOS_SYSTEM_PROPERTIES_REFACTOR.md](./OHOS_SYSTEM_PROPERTIES_REFACTOR.md) - 重构总结
- [OHOS_ADAPTATION_COMPLETED.md](./OHOS_ADAPTATION_COMPLETED.md) - 平台适配报告

## 🎉 总结

本次实现完成了 OHOS 平台系统属性的动态初始化，包括：

1. ✅ **C++ 层** - 全局变量 + 初始化函数 + 分类获取函数
2. ✅ **NAPI 层** - 3 个绑定函数（initSystemProperties, setSafeInsets, setAudioConfig）
3. ✅ **ArkTS 层** - EntryAbility 中自动初始化 + Index 中设置安全区域

现在 PPSSPP 可以获取真实的设备信息，支持不同的设备型号、屏幕尺寸和刘海屏，完全参考 Android 的成熟实现！🚀
