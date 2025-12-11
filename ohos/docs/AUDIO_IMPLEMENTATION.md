# OHOS 音频实现

## 概述

PPSSPP 的 OHOS 音频实现基于 **OHAudio API**（鸿蒙原生音频 API），参考了 Android 的 OpenSL ES 实现。

## 架构

```
PPSSPP Core
    ↓
AudioBackend (接口)
    ↓
OhosAudioBackend (实现)
    ↓
OhosAudioContext (OHAudio 封装)
    ↓
OH_AudioRenderer (鸿蒙系统 API)
```

## 文件结构

### 核心文件

1. **ohos_audio_backend.h/cpp**
   - 实现 `AudioBackend` 接口
   - 桥接 PPSSPP 和 OHAudio
   - 处理 float 到 short 的转换

2. **ohos_audio.h/cpp**
   - OHAudio API 的 C 风格封装
   - 与 Android 的 `AndroidAudio` API 兼容
   - 管理音频生命周期

3. **ohos_system_stubs.cpp**
   - 实现 `System_CreateAudioBackend()`
   - 创建 `OhosAudioBackend` 实例

## 音频流程

### 1. 初始化

```cpp
// PPSSPP 调用
AudioBackend *backend = System_CreateAudioBackend();
backend->SetRenderCallback(myCallback, userData);
backend->InitOutputDevice("default", LatencyMode::Safe, nullptr);
```

```
System_CreateAudioBackend()
    ↓
new OhosAudioBackend()
    ↓
OhosAudio_Init()
    ↓
new OhosAudioContext()
```

### 2. 启动音频

```cpp
// 在 InitOutputDevice 中
OhosAudio_Resume(audioState)
    ↓
OhosAudioContext::Init()
    ↓
OH_AudioStreamBuilder_Create()
OH_AudioStreamBuilder_SetSamplingRate()
OH_AudioStreamBuilder_SetChannelCount(2)  // 立体声
OH_AudioStreamBuilder_SetSampleFormat(S16LE)  // 16-bit PCM
OH_AudioStreamBuilder_GenerateRenderer()
OH_AudioRenderer_Start()
```

### 3. 音频回调

```
OH_AudioRenderer (系统)
    ↓ 回调
OhosAudioContext::AudioRendererCallback()
    ↓ 调用
OhosAudioBackend::AudioCallbackStatic()
    ↓ 调用
OhosAudioBackend::AudioCallback()
    ↓ 调用 renderCallback_ (float)
PPSSPP 混音器
    ↓ 输出 float 数据
OhosAudioBackend::AudioCallback()
    ↓ 转换 float → short
OH_AudioRenderer (系统播放)
```

### 4. 暂停/恢复

```cpp
// 应用进入后台
OhosAudio_Pause()
    ↓
OH_AudioRenderer_Stop()
OH_AudioRenderer_Release()

// 应用回到前台
OhosAudio_Resume()
    ↓
重新创建 AudioRenderer
OH_AudioRenderer_Start()
```

### 5. 关闭

```cpp
OhosAudio_Shutdown()
    ↓
delete OhosAudioContext
    ↓
OH_AudioRenderer_Release()
OH_AudioStreamBuilder_Destroy()
```

## 关键参数

### 采样率
- 默认：44100 Hz
- 可配置：通过 `optimalSampleRate` 参数

### 缓冲区大小
- 安全模式：512 帧
- 低延迟模式：256 帧
- 可配置：通过 `optimalFramesPerBuffer` 参数

### 声道
- 固定：2（立体声）

### 采样格式
- 固定：16-bit PCM (S16LE)

### 延迟模式
- `LatencyMode::Safe`：512 帧，约 11.6ms @ 44100Hz
- `LatencyMode::Aggressive`：256 帧，约 5.8ms @ 44100Hz

## 数据转换

### Float 到 Short

PPSSPP 内部使用 float 格式（-1.0 ~ 1.0），OHAudio 使用 short 格式（-32768 ~ 32767）：

```cpp
for (int i = 0; i < numSamples; i++) {
    float sample = mixBuffer_[i];
    sample = clamp_value(sample, -1.0f, 1.0f);  // 限幅
    buffer[i] = (short)(sample * 32767.0f);     // 转换
}
```

## 与 Android 的对比

| 特性 | Android (OpenSL ES) | OHOS (OHAudio) |
|------|---------------------|----------------|
| API | OpenSL ES | OHAudio |
| 初始化 | `slCreateEngine()` | `OH_AudioStreamBuilder_Create()` |
| 配置 | `SLDataFormat_PCM` | `OH_AudioStreamBuilder_Set*()` |
| 回调 | `SLAndroidSimpleBufferQueueItf` | `OH_AudioStreamBuilder_SetRendererCallback()` |
| 启动 | `(*bqPlayerPlay)->SetPlayState()` | `OH_AudioRenderer_Start()` |
| 停止 | `(*bqPlayerPlay)->SetPlayState()` | `OH_AudioRenderer_Stop()` |

## 线程安全

- 使用 `std::mutex` 保护回调函数指针
- 音频回调在独立线程中执行
- 避免在回调中执行耗时操作

## 错误处理

- 所有 OHAudio API 调用都检查返回值
- 初始化失败时返回 `false`
- 错误信息通过 `OhosAudio_GetErrorString()` 获取
- 使用 HiLog 和 PPSSPP Log 记录错误

## 性能优化

1. **低延迟模式**
   - 使用 `AUDIOSTREAM_LATENCY_MODE_FAST`
   - 减小缓冲区大小

2. **避免内存分配**
   - 预分配混音缓冲区
   - 回调中不分配内存

3. **最小化锁竞争**
   - 只在必要时加锁
   - 回调中快速处理

## 已知限制

1. **录音功能**
   - 暂未实现
   - API 已预留，返回 `false`

2. **设备枚举**
   - 只返回默认设备
   - OHOS 通常只有一个音频设备

3. **动态采样率切换**
   - 需要重新初始化音频
   - 暂不支持运行时切换

## 调试

### 启用日志

```cpp
// HiLog
hdc shell hilog | grep "PPSSPP_Audio"

// PPSSPP Log
查看 Log::Audio 类别
```

### 常见问题

**问题：没有声音**
- 检查 `OhosAudio_Resume()` 是否被调用
- 检查 `renderCallback_` 是否已设置
- 查看日志中的错误信息

**问题：声音卡顿**
- 增大缓冲区大小（512 → 1024）
- 检查 CPU 使用率
- 使用安全模式而非低延迟模式

**问题：延迟太高**
- 减小缓冲区大小（512 → 256）
- 使用低延迟模式
- 检查系统音频设置

## 测试

### 基本测试

1. 启动 PPSSPP
2. 加载游戏
3. 确认有声音输出
4. 调整音量
5. 暂停/恢复应用

### 压力测试

1. 长时间运行（1小时+）
2. 频繁暂停/恢复
3. 切换不同游戏
4. 调整音频设置

## 未来改进

1. **录音支持**
   - 实现 `OH_AudioCapturer`
   - 支持麦克风输入

2. **设备选择**
   - 枚举所有音频设备
   - 支持蓝牙耳机等

3. **动态参数调整**
   - 运行时切换采样率
   - 动态调整缓冲区大小

4. **音效处理**
   - 均衡器
   - 混响
   - 3D 音效

## 参考资料

- [OHAudio API 文档](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/ohaudio-overview-V5)
- Android OpenSL ES 实现：`android/jni/OpenSLContext.cpp`
- PPSSPP AudioBackend 接口：`Common/Audio/AudioBackend.h`

## 相关文件

- `ohos/entry/src/main/cpp/ohos_audio_backend.h`
- `ohos/entry/src/main/cpp/ohos_audio_backend.cpp`
- `ohos/entry/src/main/cpp/ohos_audio.h`
- `ohos/entry/src/main/cpp/ohos_audio.cpp`
- `ohos/entry/src/main/cpp/ohos_system_stubs.cpp`
- `ohos/entry/src/main/cpp/CMakeLists.txt`
