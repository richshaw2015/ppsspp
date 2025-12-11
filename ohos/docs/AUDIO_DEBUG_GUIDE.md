# OHOS 音频调试指南

## 问题：按钮没有声音

音频系统已初始化，但输出的是静音数据。

## 调试步骤

### 1. 检查音频系统初始化

```bash
hdc shell hilog -x | grep -E "NativeInit|System_CreateAudioBackend|OhosAudioBackend"
```

**预期输出**：
```
[PPSSPP_NAPI] Calling NativeInit...
[System] System_CreateAudioBackend: creating OhosAudioBackend
[Audio] OhosAudioBackend created
[PPSSPP_Audio] OhosAudioContext created
[PPSSPP_Audio] OHAudio renderer started
[PPSSPP_NAPI] NativeInit completed
```

✅ **已确认**：音频系统初始化成功

### 2. 检查音频回调

```bash
hdc shell hilog -x | grep "AudioCallback"
```

**预期输出**：
```
[Audio] AudioCallback called 1000 times, renderCallback_=0x...
[Audio] Max sample value: 0.1234
```

**可能的问题**：

#### 问题 A：renderCallback_ = 0x0 (nullptr)
```
[Audio] AudioCallback called 1000 times, renderCallback_=0x0
[Audio] No render callback set! Outputting silence.
```

**原因**：`SetRenderCallback()` 没有被调用

**解决方案**：检查 `NativeInit()` 中是否调用了 `SetRenderCallback()`

#### 问题 B：Max sample value = 0.0000
```
[Audio] AudioCallback called 1000 times, renderCallback_=0x12345678
[Audio] Max sample value: 0.0000
```

**原因**：PPSSPP 混音器没有产生音频数据

**可能的原因**：
1. 音频设置中音量为 0
2. 按键音效未启用
3. 混音器未正确初始化
4. 没有音频源（游戏未运行）

### 3. 检查 PPSSPP 音频设置

在 PPSSPP 中：
1. 进入 设置 → 音频
2. 检查：
   - 音量：应该 > 0
   - 按键音效：应该启用
   - 音频后端：应该显示 "OHAudio"

### 4. 检查系统音频

```bash
hdc shell hilog -x | grep "IncSilentData"
```

**当前输出**：
```
[IncSilentData][[100080]RendererInClient], channel: 2, counts: 1000
[ReportWriteMuteEvent][100080]MutePlaying for 30014 ms, muteState:0
```

这表明系统检测到应用在播放静音数据。

### 5. 检查 NativeMix 回调

让我们确认 `NativeMix` 是否被正确设置为回调函数。

在 `UI/NativeApp.cpp` 中应该有：
```cpp
g_audioBackend->SetRenderCallback(&NativeMixWrapper, nullptr);
```

### 6. 添加更多调试日志

在 `ohos_audio_backend.cpp` 中已添加：
- 每 1000 次回调打印一次
- 显示 `renderCallback_` 指针
- 显示最大样本值

### 7. 检查音频格式

确认音频参数匹配：
- 采样率：44100 Hz
- 声道：2 (立体声)
- 格式：16-bit PCM
- 缓冲区：256 或 512 帧

## 常见问题和解决方案

### 问题 1：renderCallback_ 未设置

**症状**：
```
[Audio] No render callback set! Outputting silence.
```

**检查**：
```bash
hdc shell hilog -x | grep "SetRenderCallback"
```

**解决方案**：
确保 `NativeInit()` 中调用了：
```cpp
g_audioBackend->SetRenderCallback(&NativeMixWrapper, nullptr);
```

### 问题 2：混音器产生静音

**症状**：
```
[Audio] Max sample value: 0.0000
```

**可能原因**：
1. **音量设置为 0**
   - 检查 PPSSPP 设置 → 音频 → 音量
   
2. **按键音效未启用**
   - 检查 PPSSPP 设置 → 音频 → 按键音效
   
3. **没有音频源**
   - 按键音效需要在 UI 中点击按钮才会播放
   - 游戏音频需要加载游戏后才有

4. **混音器未初始化**
   - 检查 `NativeInit()` 是否完整执行

### 问题 3：音频延迟

**症状**：声音延迟明显

**解决方案**：
- 减小缓冲区大小（512 → 256 帧）
- 使用低延迟模式

### 问题 4：音频卡顿

**症状**：声音断断续续

**解决方案**：
- 增大缓冲区大小（256 → 512 帧）
- 使用安全模式

## 下一步调试

### 1. 重新编译

```bash
cd ohos
./rebuild.sh
```

### 2. 运行并查看日志

```bash
# 启动应用后
hdc shell hilog -x | grep -E "Audio|PPSSPP"
```

### 3. 点击按钮

在 PPSSPP UI 中点击任意按钮，观察日志。

### 4. 分析日志

查找：
- `renderCallback_` 的值（应该不是 0x0）
- `Max sample value` 的值（应该 > 0）

## 预期结果

**正常工作时的日志**：
```
[PPSSPP_Audio] OHAudio renderer started
[Audio] AudioCallback called 1000 times, renderCallback_=0x12345678
[Audio] Max sample value: 0.1234
```

**点击按钮时**：
```
[Audio] Max sample value: 0.5678  ← 应该看到非零值
```

## 临时测试：强制输出测试音

如果想测试音频输出是否工作，可以临时修改回调函数输出测试音：

```cpp
void OhosAudioBackend::AudioCallback(short *buffer, int numSamples, int sampleRateHz) {
    // 临时测试：输出 440Hz 正弦波
    static float phase = 0.0f;
    float freq = 440.0f;  // A4 音符
    float delta = 2.0f * M_PI * freq / sampleRateHz;
    
    for (int i = 0; i < numSamples; i += 2) {
        float sample = sin(phase) * 0.3f;  // 30% 音量
        buffer[i] = (short)(sample * 32767.0f);      // 左声道
        buffer[i+1] = (short)(sample * 32767.0f);    // 右声道
        phase += delta;
        if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
    }
}
```

如果能听到 440Hz 的音调，说明音频输出路径是正常的，问题在于 PPSSPP 混音器。

## 相关文件

- `ohos/entry/src/main/cpp/ohos_audio_backend.cpp` - 音频回调
- `UI/NativeApp.cpp` - NativeInit 和 SetRenderCallback
- `Common/Audio/AudioBackend.h` - AudioBackend 接口
- `ohos/AUDIO_IMPLEMENTATION.md` - 实现文档
