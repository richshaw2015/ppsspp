# 音频测试音调试

## 目的

通过输出测试音（440Hz 正弦波）来验证音频输出路径是否正常工作。

## 临时修改

在 `ohos/entry/src/main/cpp/ohos_audio_backend.cpp` 的 `AudioCallback` 函数中添加测试音输出。

### 方法 1：完全替换为测试音

```cpp
void OhosAudioBackend::AudioCallback(short *buffer, int numSamples, int sampleRateHz) {
    // 临时测试：输出 440Hz 正弦波（A4 音符）
    static float phase = 0.0f;
    float freq = 440.0f;
    float delta = 2.0f * M_PI * freq / sampleRateHz;
    
    for (int i = 0; i < numSamples; i += 2) {
        float sample = sin(phase) * 0.3f;  // 30% 音量
        buffer[i] = (short)(sample * 32767.0f);      // 左声道
        buffer[i+1] = (short)(sample * 32767.0f);    // 右声道
        phase += delta;
        if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
    }
    
    INFO_LOG(Log::Audio, "Test tone output: 440Hz sine wave");
}
```

### 方法 2：混合测试音和 PPSSPP 音频

```cpp
void OhosAudioBackend::AudioCallback(short *buffer, int numSamples, int sampleRateHz) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 先获取 PPSSPP 音频
    if (renderCallback_) {
        int numFrames = numSamples / 2;
        renderCallback_(mixBuffer_, numFrames, sampleRateHz, renderUserdata_);
        
        // 检查是否有音频数据
        float maxSample = 0.0f;
        for (int i = 0; i < numSamples; i++) {
            maxSample = std::max(maxSample, fabs(mixBuffer_[i]));
        }
        
        if (maxSample > 0.001f) {
            // 有音频数据，正常输出
            for (int i = 0; i < numSamples; i++) {
                float sample = clamp_value(mixBuffer_[i], -1.0f, 1.0f);
                buffer[i] = (short)(sample * 32767.0f);
            }
            INFO_LOG(Log::Audio, "PPSSPP audio output: max=%.4f", maxSample);
        } else {
            // 没有音频数据，输出测试音
            static float phase = 0.0f;
            float freq = 440.0f;
            float delta = 2.0f * M_PI * freq / sampleRateHz;
            
            for (int i = 0; i < numSamples; i += 2) {
                float sample = sin(phase) * 0.1f;  // 10% 音量（较小）
                buffer[i] = (short)(sample * 32767.0f);
                buffer[i+1] = (short)(sample * 32767.0f);
                phase += delta;
                if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
            }
        }
    } else {
        // 没有回调，输出测试音
        static float phase = 0.0f;
        float freq = 440.0f;
        float delta = 2.0f * M_PI * freq / sampleRateHz;
        
        for (int i = 0; i < numSamples; i += 2) {
            float sample = sin(phase) * 0.3f;
            buffer[i] = (short)(sample * 32767.0f);
            buffer[i+1] = (short)(sample * 32767.0f);
            phase += delta;
            if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
        }
        
        WARN_LOG(Log::Audio, "No render callback - test tone output");
    }
}
```

## 测试步骤

1. **修改代码**：选择上面的方法 1 或方法 2
2. **重新编译**：`cd ohos && ./rebuild.sh`
3. **安装应用**：`hdc install entry/build/default/outputs/default/entry-default-signed.hap`
4. **启动应用**：在设备上打开 PPSSPP
5. **听声音**：
   - 方法 1：应该立即听到持续的 440Hz 音调
   - 方法 2：如果 PPSSPP 没有音频，会听到较小的 440Hz 音调

## 预期结果

### 如果能听到测试音

✅ **音频输出路径正常**

问题在于 PPSSPP 混音器：
- `renderCallback_` 可能未设置
- 混音器产生的是静音数据
- 音频设置问题（音量为 0、按键音效未启用）

### 如果听不到测试音

❌ **音频输出路径有问题**

可能的原因：
- OHAudio 初始化失败
- 音频流未启动
- 系统音频权限问题
- 设备音量为 0

## 恢复正常代码

测试完成后，记得恢复原来的代码（使用 git）：

```bash
cd ohos/entry/src/main/cpp
git checkout ohos_audio_backend.cpp
```

或者手动删除测试音代码，恢复正常的混音逻辑。

## 相关文件

- `ohos/entry/src/main/cpp/ohos_audio_backend.cpp` - 音频回调函数
- `ohos/AUDIO_DEBUG_GUIDE.md` - 调试指南

