# OHOS 音频调试 - 最终状态

## 已完成的改进

### 1. 统一使用 OHOS_LOG 格式

所有 `ohos_audio_backend.cpp` 中的日志现在使用：
- `OHOS_LOGI` 替代 `INFO_LOG`
- `OHOS_LOGW` 替代 `WARN_LOG`
- `OHOS_LOGE` 替代 `ERROR_LOG`

### 2. 添加 %{public} 标记

所有日志格式字符串都添加了 `%{public}` 标记：
- `%{public}d` - 整数
- `%{public}f` - 浮点数
- `%{public}s` - 字符串
- `%{public}p` - 指针

这样可以绕过鸿蒙的隐私保护机制，确保日志值能够正常显示。

### 3. 增强的调试日志

**SetRenderCallback**：
```cpp
OHOS_LOGI(AUDIO_BACKEND_TAG, "SetRenderCallback: callback=%{public}p, userdata=%{public}p", 
          callback, userdata);
```

**AudioCallback**（每 100 次打印一次）：
```cpp
OHOS_LOGI(AUDIO_BACKEND_TAG, "AudioCallback #%{public}d: renderCallback_=%{public}p, numSamples=%{public}d", 
          callCount, renderCallback_, numSamples);
OHOS_LOGI(AUDIO_BACKEND_TAG, "Max sample value: %{public}.4f (numFrames=%{public}d, rate=%{public}d)", 
          maxSample, numFrames, sampleRateHz);
```

### 4. 更新的调试脚本

- `quick_test.sh` - 一键测试脚本
- `debug_audio.sh` - 音频日志监控
- 都已更新以匹配新的日志标签 `PPSSPP_AudioBackend`

## 现在可以看到的日志

运行 `./quick_test.sh` 后，应该能看到：

```
[PPSSPP_AudioBackend] OhosAudioBackend created
[PPSSPP_AudioBackend] InitOutputDevice: uniqueId=default, latencyMode=0
[PPSSPP_Audio] OhosAudio_Init: frames=256, rate=44100
[PPSSPP_Audio] OHAudio renderer started
[PPSSPP_AudioBackend] Audio output device initialized: rate=44100, buffer=256
[PPSSPP_AudioBackend] SetRenderCallback: callback=0x..., userdata=0x0
[PPSSPP_AudioBackend] AudioCallback #100: renderCallback_=0x..., numSamples=512
[PPSSPP_AudioBackend] Max sample value: 0.0000 (numFrames=256, rate=44100)
```

## 关键诊断信息

### 1. SetRenderCallback 日志

**正常**：`callback=0x12345678`（非零地址）
**异常**：`callback=0x0` 或没有这条日志

### 2. AudioCallback 日志

**正常**：`renderCallback_=0x12345678`（与 SetRenderCallback 的地址相同）
**异常**：`renderCallback_=0x0` 或 "No render callback set!"

### 3. Max sample 值

**初始状态**：`0.0000`（正常，因为没有音频源）
**点击按钮后**：应该变成非零值（如 `0.1234`）
**异常**：始终是 `0.0000`（说明混音器没有产生音频）

## 下一步操作

### 立即执行

```bash
cd ohos
./quick_test.sh
```

### 在应用中测试

1. 启动 PPSSPP
2. 进入 设置 → 音频
3. 确认：
   - 音量 > 0
   - 按键音效已启用
4. 返回主菜单
5. 点击任意按钮
6. 观察日志中的 "Max sample value"

### 预期结果

点击按钮时，应该看到：
```
[PPSSPP_AudioBackend] Max sample value: 0.1234 (numFrames=256, rate=44100)
```

如果值仍然是 `0.0000`，说明 PPSSPP 混音器没有产生音频数据。

## 可能的问题和解决方案

### 问题 1：没有看到 SetRenderCallback 日志

**原因**：`NativeInitGraphics` 没有完整执行

**检查**：
```bash
hdc shell hilog -x | grep "NativeInitGraphics"
```

### 问题 2：callback 或 renderCallback_ 是 0x0

**原因**：回调函数指针为空

**检查**：`NativeMixWrapper` 函数是否正确定义

### 问题 3：Max sample 始终为 0

**原因**：PPSSPP 混音器没有产生音频

**可能的原因**：
1. 音量设置为 0
2. 按键音效未启用
3. 没有触发音频事件（需要点击按钮）
4. PPSSPP Core 音频系统未正确初始化

**解决方案**：
- 检查 PPSSPP 音频设置
- 确保在 UI 中点击按钮
- 如果仍然不行，尝试输出测试音（见 `AUDIO_TEST_TONE.md`）

## 测试音验证

如果怀疑是 PPSSPP 混音器的问题，可以临时输出测试音。

在 `ohos_audio_backend.cpp` 的 `AudioCallback` 函数开头添加：

```cpp
// 临时测试：输出 440Hz 正弦波
static float phase = 0.0f;
float freq = 440.0f;
float delta = 2.0f * M_PI * freq / sampleRateHz;

for (int i = 0; i < numSamples; i += 2) {
    float sample = sin(phase) * 0.3f;  // 30% 音量
    buffer[i] = (short)(sample * 32767.0f);
    buffer[i+1] = (short)(sample * 32767.0f);
    phase += delta;
    if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
}
OHOS_LOGI(AUDIO_BACKEND_TAG, "Test tone output");
return;  // 跳过正常的混音逻辑
```

如果能听到 440Hz 的音调，说明音频输出路径正常。

## 相关文档

- `AUDIO_TROUBLESHOOTING.md` - 详细的问题排查指南
- `AUDIO_NEXT_STEPS.md` - 操作步骤说明
- `AUDIO_TEST_TONE.md` - 测试音输出指南
- `AUDIO_DEBUG_GUIDE.md` - 调试指南

## 快速命令

```bash
# 一键测试
cd ohos && ./quick_test.sh

# 只监控音频日志
cd ohos && ./debug_audio.sh

# 手动查看日志
hdc shell hilog -x | grep AudioBackend

# 查看 SetRenderCallback
hdc shell hilog -x | grep SetRenderCallback

# 查看音频数据
hdc shell hilog -x | grep "Max sample"
```

