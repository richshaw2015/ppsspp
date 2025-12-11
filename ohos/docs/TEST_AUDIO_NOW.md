# 立即测试音频

## 已修复的问题

✅ **缓冲区大小问题**：现在会自动扩展到 8192 样本

## 当前状态

从日志可以看到：
- ✅ SetRenderCallback 成功（callback=0x5c785172ec）
- ✅ AudioCallback 正常调用
- ❓ Max sample = 0.0000（需要测试）

## 立即执行的步骤

### 1. 重新编译并安装

```bash
cd ohos
./rebuild.sh
hdc install entry/build/default/outputs/default/entry-default-signed.hap
```

### 2. 启动应用并测试

1. 在设备上启动 PPSSPP

2. 进入 **设置 → 音频**，确认：
   - **音量** > 0（建议设置为 5 或更高）
   - **按键音效** 已启用（打勾）

3. 返回主菜单

4. **点击任意按钮**（如 "设置"、"游戏"、"最近" 等）

5. 同时在电脑上监控日志：
   ```bash
   hdc shell hilog -x | grep "Max sample"
   ```

### 3. 预期结果

**点击按钮前**：
```
Max sample value: 0.0000
```

**点击按钮时**：
```
Max sample value: 0.1234  ← 应该变成非零值
```

**并且应该能听到按键音效**

## 如果仍然是 0.0000

### 选项 1：检查设置

1. 确认音量不是 0
2. 确认按键音效已启用
3. 尝试调高音量到最大

### 选项 2：输出测试音

如果确认设置正确但仍然没有声音，可以输出测试音来验证音频路径。

编辑 `ohos/entry/src/main/cpp/ohos_audio_backend.cpp`，在 `AudioCallback` 函数中，找到这一行：

```cpp
// 调用 PPSSPP 的渲染回调（输出 float）
renderCallback_(mixBuffer_, numFrames, sampleRateHz, renderUserdata_);
```

在它**之后**添加：

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

然后重新编译和安装。如果能听到持续的 440Hz 音调，说明音频输出正常，问题在于 PPSSPP 混音器。

## 快速命令

```bash
# 重新编译
cd ohos && ./rebuild.sh

# 安装
hdc install entry/build/default/outputs/default/entry-default-signed.hap

# 监控日志
hdc shell hilog -x | grep "Max sample"

# 查看完整音频日志
hdc shell hilog -x | grep AudioBackend
```

## 关键点

1. **必须在 UI 中点击按钮**才会触发按键音效
2. **必须启用按键音效**（在设置中）
3. **音量必须 > 0**

如果这些都确认了但仍然没有声音，那可能是 PPSSPP Core 的音频系统配置问题。

