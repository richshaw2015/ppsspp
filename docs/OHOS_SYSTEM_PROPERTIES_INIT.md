# OHOS 系统属性动态初始化

## 概述

参考 Android 的实现，OHOS 平台的系统属性现在使用**全局变量**存储，并通过 ArkTS 层动态初始化，而不是硬编码固定值。

## 改进对比

### ❌ 之前的实现（硬编码）
```cpp
std::string System_GetProperty(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_NAME:
            return "HarmonyOS";  // 硬编码
        case SYSPROP_DISPLAY_XRES:
            return "1080";       // 硬编码
        // ...
    }
}
```

### ✅ 现在的实现（动态）
```cpp
// 全局变量
static std::string systemName = "HarmonyOS";
static int display_xres = 1080;

// 初始化函数（从 ArkTS 调用）
void OhosSystemProperties_Init(...) {
    systemName = deviceName;
    display_xres = xres;
    // ...
}

// 获取属性
std::string System_GetProperty(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_NAME:
            return systemName;  // 返回动态值
        // ...
    }
}

int64_t System_GetPropertyInt(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_DISPLAY_XRES:
            return display_xres;  // 返回动态值
        // ...
    }
}
```

## 需要初始化的属性

### 1. 设备信息
- **systemName**: 设备名称（如 "HUAWEI Mate 60"）
- **systemBuild**: 系统版本（如 "HarmonyOS NEXT 5.0.0"）
- **boardName**: 主板名称
- **cpuInfo**: CPU 信息

### 2. 系统配置
- **systemVersion**: 系统版本号（整数）
- **deviceType**: 设备类型（0=手机, 1=平板, 2=电视）
- **langRegion**: 语言区域（如 "zh_CN", "en_US"）

### 3. 显示属性
- **display_xres**: 屏幕宽度（像素）
- **display_yres**: 屏幕高度（像素）
- **display_dpi**: 屏幕 DPI
- **display_refresh_rate**: 刷新率（Hz）

### 4. 安全区域（刘海屏）
- **g_safeInsetLeft**: 左侧安全区域
- **g_safeInsetTop**: 顶部安全区域
- **g_safeInsetRight**: 右侧安全区域
- **g_safeInsetBottom**: 底部安全区域

### 5. 音频配置
- **sampleRate**: 采样率
- **framesPerBuffer**: 缓冲区帧数
- **optimalSampleRate**: 最佳采样率
- **optimalFramesPerBuffer**: 最佳缓冲区帧数

## ArkTS 层调用示例

### 在 EntryAbility.ets 中初始化

```typescript
import { ppsspp } from 'libppsspp.so';
import { deviceInfo } from '@kit.BasicServicesKit';
import { display } from '@kit.ArkUI';

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
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
    let deviceType = 0;  // 0=手机
    if (deviceInfo.deviceType === 'tablet') {
      deviceType = 1;  // 平板
    } else if (deviceInfo.deviceType === 'tv') {
      deviceType = 2;  // 电视
    }
    
    // 5. 调用 C++ 初始化函数
    ppsspp.initSystemProperties(
      deviceName,
      deviceBuild,
      langRegion,
      osVersion,
      deviceType,
      xres,
      yres,
      dpi,
      refreshRate
    );
    
    console.info('[PPSSPP] System properties initialized');
    console.info(`  Device: ${deviceName} (${deviceBuild})`);
    console.info(`  Language: ${langRegion}`);
    console.info(`  Display: ${xres}x${yres} @${dpi}DPI ${refreshRate}Hz`);
  }
}
```

### 设置安全区域

```typescript
// 在 Index.ets 的 onAreaChange 回调中
.onAreaChange((oldValue: Area, newValue: Area) => {
  // 获取安全区域
  const safeArea = this.getUIContext().getSafeAreaInsets();
  
  // 设置到 C++ 层
  ppsspp.setSafeInsets(
    safeArea.left,
    safeArea.top,
    safeArea.right,
    safeArea.bottom
  );
  
  console.info(`[PPSSPP] Safe insets: L=${safeArea.left} T=${safeArea.top} R=${safeArea.right} B=${safeArea.bottom}`);
})
```

### 设置音频配置

```typescript
import { audio } from '@kit.AudioKit';

// 获取音频配置
const audioManager = audio.getAudioManager();
const audioRenderer = await audio.createAudioRenderer({
  streamInfo: {
    samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_48000,
    channels: audio.AudioChannel.CHANNEL_2,
    sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
    encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
  },
  rendererInfo: {
    usage: audio.StreamUsage.STREAM_USAGE_GAME,
    rendererFlags: 0
  }
});

const bufferSize = await audioRenderer.getBufferSize();
const sampleRate = 48000;
const framesPerBuffer = bufferSize / 4;  // 16-bit stereo

// 设置到 C++ 层
ppsspp.setAudioConfig(
  sampleRate,
  framesPerBuffer,
  sampleRate,  // optimal
  framesPerBuffer  // optimal
);

console.info(`[PPSSPP] Audio config: ${sampleRate}Hz ${framesPerBuffer} frames`);
```

## NAPI 绑定示例

在 `napi_ppsspp.cpp` 中添加绑定：

```cpp
// 初始化系统属性
static napi_value InitSystemProperties(napi_env env, napi_callback_info info) {
    size_t argc = 9;
    napi_value args[9];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 9) {
        napi_throw_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }
    
    // 获取参数
    char deviceName[256], deviceBuild[256], language[32];
    size_t len;
    napi_get_value_string_utf8(env, args[0], deviceName, sizeof(deviceName), &len);
    napi_get_value_string_utf8(env, args[1], deviceBuild, sizeof(deviceBuild), &len);
    napi_get_value_string_utf8(env, args[2], language, sizeof(language), &len);
    
    int32_t osVersion, devType, xres, yres, dpi;
    double refreshRate;
    napi_get_value_int32(env, args[3], &osVersion);
    napi_get_value_int32(env, args[4], &devType);
    napi_get_value_int32(env, args[5], &xres);
    napi_get_value_int32(env, args[6], &yres);
    napi_get_value_int32(env, args[7], &dpi);
    napi_get_value_double(env, args[8], &refreshRate);
    
    // 调用初始化函数
    OhosSystemProperties_Init(
        deviceName, deviceBuild, language,
        osVersion, devType,
        xres, yres, dpi, (float)refreshRate
    );
    
    return nullptr;
}

// 设置安全区域
static napi_value SetSafeInsets(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    double left, top, right, bottom;
    napi_get_value_double(env, args[0], &left);
    napi_get_value_double(env, args[1], &top);
    napi_get_value_double(env, args[2], &right);
    napi_get_value_double(env, args[3], &bottom);
    
    OhosSystemProperties_SetSafeInsets(
        (float)left, (float)top, (float)right, (float)bottom
    );
    
    return nullptr;
}

// 设置音频配置
static napi_value SetAudioConfig(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    int32_t rate, frames, optRate, optFrames;
    napi_get_value_int32(env, args[0], &rate);
    napi_get_value_int32(env, args[1], &frames);
    napi_get_value_int32(env, args[2], &optRate);
    napi_get_value_int32(env, args[3], &optFrames);
    
    OhosSystemProperties_SetAudioConfig(rate, frames, optRate, optFrames);
    
    return nullptr;
}

// 在模块初始化中注册
EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    // ... 其他绑定 ...
    
    napi_property_descriptor desc[] = {
        // ... 其他属性 ...
        { "initSystemProperties", nullptr, InitSystemProperties, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setSafeInsets", nullptr, SetSafeInsets, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setAudioConfig", nullptr, SetAudioConfig, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END
```

## 优势

### 1. 动态适配
- ✅ 支持不同设备型号
- ✅ 支持不同屏幕分辨率
- ✅ 支持不同系统版本

### 2. 准确性
- ✅ 真实的设备信息
- ✅ 真实的屏幕参数
- ✅ 真实的音频配置

### 3. 可维护性
- ✅ 与 Android 实现一致
- ✅ 易于理解和修改
- ✅ 便于调试

## 注意事项

1. **初始化时机**: 必须在 PPSSPP 核心初始化之前调用
2. **线程安全**: 初始化函数应该在主线程调用
3. **参数验证**: ArkTS 层应该验证参数的有效性
4. **默认值**: C++ 层提供合理的默认值，即使未初始化也能工作

## 测试建议

1. 在不同设备上测试（手机、平板）
2. 测试不同分辨率和 DPI
3. 测试刘海屏设备的安全区域
4. 验证日志输出的系统信息
5. 确保属性值正确传递到 PPSSPP 核心

## 相关文件

- `ohos_system_properties.cpp` - 系统属性实现
- `ohos_system.h` - 头文件声明
- `napi_ppsspp.cpp` - NAPI 绑定
- `EntryAbility.ets` - ArkTS 初始化代码
