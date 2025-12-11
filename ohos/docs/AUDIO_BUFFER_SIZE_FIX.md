# OHOS 音频缓冲区大小问题修复

## 问题描述

日志显示：
```
Mix buffer too small: 512 < 8192
```

**原因**：
- 我们请求 256 帧（512 样本）的缓冲区
- OHAudio 实际使用 4096 帧（8192 样本）的缓冲区
- 导致缓冲区溢出

## 根本原因

OHAudio 的 `OH_AudioStreamBuilder_SetFramesPerCallback` 只是一个**建议值**，系统可能会使用不同的缓冲区大小。实际的缓冲区大小在回调时才能确定。

## 解决方案

动态扩展混音缓冲区以适应实际的回调大小：

```cpp
// 动态扩展混音缓冲区（如果需要）
if (numSamples > mixBufferSize_) {
    OHOS_LOGI(AUDIO_BACKEND_TAG, "Expanding mix buffer: %{public}d -> %{public}d", 
              mixBufferSize_, numSamples);
    delete[] mixBuffer_;
    mixBufferSize_ = numSamples;
    mixBuffer_ = new float[mixBufferSize_];
}
```

这样，第一次回调时会自动扩展缓冲区到 8192 样本，之后就不会再有警告。

## 当前状态

从日志可以看到：

### ✅ 已修复的问题

1. **SetRenderCallback 成功**：
   ```
   SetRenderCallback: callback=0x5c785172ec, userdata=(nil)
   ```
   回调指针正常（非零）

2. **AudioCallback 正常调用**：
   ```
   AudioCallback #100: renderCallback_=0x5c785172ec, numSamples=8192
   ```
   回调被正常调用，指针正确

3. **缓冲区大小问题**：
   已通过动态扩展解决

### ❌ 仍然存在的问题

**Max sample value: 0.0000**

这说明 PPSSPP 混音器没有产生音频数据。

## 为什么没有音频数据？

可能的原因：

### 1. 没有音频源

按键音效只在以下情况下播放：
- 用户在 PPSSPP UI 中点击按钮
- 按键音效已在设置中启用
- 音量 > 0

**测试方法**：
1. 进入 PPSSPP 设置 → 音频
2. 确认音量 > 0
3. 确认按键音效已启用
4. 返回主菜单
5. **点击任意按钮**
6. 观察日志中的 "Max sample value"

### 2. PPSSPP 音频系统未完全初始化

虽然 `SetRenderCallback` 被调用了，但 PPSSPP 内部的音频系统可能还没有完全初始化。

**检查方法**：
查看是否有 PPSSPP Core 的音频初始化日志。

### 3. 音频混音器配置问题

PPSSPP 的混音器可能需要特定的配置才能工作。

## 下一步测试

### 方法 1：在 UI 中点击按钮

1. 重新编译并安装：
   ```bash
   cd ohos
   ./quick_test.sh
   ```

2. 启动 PPSSPP

3. 进入设置 → 音频，确认：
   - 音量 > 0
   - 按键音效已启用

4. 返回主菜单

5. **点击任意按钮**（如 "设置"、"游戏" 等）

6. 观察日志：
   ```bash
   hdc shell hilog -x | grep "Max sample"
   ```

   **预期**：点击按钮时，Max sample 应该变成非零值（如 0.1234）

### 方法 2：输出测试音

如果点击按钮后仍然是 0.0000，可以输出测试音来验证音频路径：

在 `ohos_audio_backend.cpp` 的 `AudioCallback` 函数中，在调用 `renderCallback_` **之后**添加：

```cpp
// 临时测试：如果 PPSSPP 没有音频，输出测试音
if (maxSample < 0.001f) {
    static float phase = 0.0f;
    float freq = 440.0f;
    float delta = 2.0f * M_PI * freq / sampleRateHz;
    
    for (int i = 0; i < numSamples; i += 2) {
        float sample = sin(phase) * 0.1f;  // 10% 音量
        buffer[i] = (short)(sample * 32767.0f);
        buffer[i+1] = (short)(sample * 32767.0f);
        phase += delta;
        if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
    }
    if (shouldLog) {
        OHOS_LOGI(AUDIO_BACKEND_TAG, "Test tone output (PPSSPP silent)");
    }
    return;
}
```

如果能听到 440Hz 的音调，说明音频输出路径正常，问题在于 PPSSPP 混音器。

## 预期结果

### 正常情况

点击按钮后，日志应该显示：
```
[PPSSPP_AudioBackend] Max sample value: 0.1234 (numFrames=4096, rate=44100)
```

并且能听到按键音效。

### 异常情况

如果仍然是 `0.0000`，说明：
1. 按键音效未启用
2. 音量为 0
3. PPSSPP 音频系统有问题

## 相关日志分析

从你的日志可以看到：

```
latencyMode=1
```

这是 `LatencyMode::Aggressive`（值为 1），说明使用的是低延迟模式。但实际缓冲区大小是 4096 帧，远大于我们请求的 256 帧。这是 OHAudio 的正常行为。

## 总结

1. ✅ **缓冲区大小问题已修复**：动态扩展缓冲区
2. ✅ **回调设置正常**：SetRenderCallback 成功
3. ✅ **回调被调用**：AudioCallback 正常工作
4. ❓ **音频数据为零**：需要在 UI 中点击按钮测试

下一步：重新编译，启动应用，在 UI 中点击按钮，观察 Max sample 值的变化。

