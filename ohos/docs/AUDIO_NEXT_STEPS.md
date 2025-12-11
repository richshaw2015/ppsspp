# OHOS 音频调试 - 下一步操作

## 当前情况

音频系统已初始化，但按钮没有声音。从日志可以看到：
- ✅ `OhosAudio_Init` 成功
- ✅ OHAudio renderer 启动
- ❓ 但没有看到 `SetRenderCallback` 或 `AudioCallback` 的日志

## 已完成的改进

### 1. 增强了日志输出

在 `ohos_audio_backend.cpp` 中：
- `SetRenderCallback`: 现在会打印回调指针地址
- `AudioCallback`: 每 100 次（而不是 1000 次）打印一次详细信息
- 包括：回调指针、样本数、最大样本值

### 2. 创建了调试工具

- `ohos/quick_test.sh` - 一键重新编译、安装、监控日志
- `ohos/debug_audio.sh` - 监控音频相关日志
- `ohos/AUDIO_TROUBLESHOOTING.md` - 详细的问题排查指南
- `ohos/AUDIO_TEST_TONE.md` - 测试音输出指南

## 立即执行的步骤

### 方法 1：使用快速测试脚本（推荐）

```bash
cd ohos
./quick_test.sh
```

这个脚本会：
1. 重新编译项目
2. 安装到设备
3. 清空日志
4. 等待你启动应用
5. 自动监控关键日志

### 方法 2：手动执行

```bash
cd ohos

# 1. 重新编译
./rebuild.sh

# 2. 安装
hdc install entry/build/default/outputs/default/entry-default-signed.hap

# 3. 清空日志
hdc shell hilog -r

# 4. 在设备上启动 PPSSPP

# 5. 查看日志
hdc shell hilog -x | grep -E "SetRenderCallback|AudioCallback|Max sample"
```

## 预期看到的日志

### 正常情况

```
[Audio] SetRenderCallback: callback=0x12345678, userdata=0x0
[Audio] AudioCallback #100: renderCallback_=0x12345678, numSamples=512
[Audio] Max sample value: 0.0000 (numFrames=256, rate=44100)
```

然后在 PPSSPP UI 中点击按钮，应该看到：

```
[Audio] Max sample value: 0.1234 (numFrames=256, rate=44100)
```

### 异常情况

#### 情况 A：没有看到任何 Audio 日志

**问题**：音频系统没有初始化

**检查**：
```bash
hdc shell hilog -x | grep "NativeInitGraphics"
```

应该看到 `NativeInitGraphics` 和 `System_CreateAudioBackend`

#### 情况 B：看到 SetRenderCallback，但 callback=0x0

**问题**：回调指针为空

**原因**：`NativeMixWrapper` 函数地址获取失败

#### 情况 C：Max sample 始终为 0

**问题**：PPSSPP 混音器没有产生音频

**检查**：
1. PPSSPP 设置 → 音频 → 音量（应该 > 0）
2. PPSSPP 设置 → 音频 → 按键音效（应该启用）
3. 在 UI 中点击按钮（触发按键音效）

## 如果仍然没有声音

### 选项 1：输出测试音

验证音频输出路径是否正常工作。

编辑 `ohos/entry/src/main/cpp/ohos_audio_backend.cpp`，在 `AudioCallback` 函数开头添加：

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
return;  // 跳过正常的混音逻辑
```

然后重新编译和安装。如果能听到持续的音调，说明音频输出正常，问题在于 PPSSPP 混音器。

### 选项 2：检查 PPSSPP 配置

在 PPSSPP 中：
1. 进入 设置 (Settings)
2. 选择 音频 (Audio)
3. 确认：
   - 音量 > 0
   - 按键音效已启用
   - 音频后端显示正确

### 选项 3：检查设备音量

确保设备音量不是 0，并且没有静音。

## 报告问题时需要的信息

如果问题仍然存在，请提供：

1. **完整的日志**：
   ```bash
   hdc shell hilog -x | grep -E "Audio|PPSSPP" > audio_log.txt
   ```

2. **SetRenderCallback 日志**：
   - 是否看到这条日志？
   - callback 指针的值是什么？

3. **AudioCallback 日志**：
   - 是否看到这条日志？
   - renderCallback_ 指针的值是什么？
   - Max sample 的值是什么？

4. **操作步骤**：
   - 是否在 UI 中点击了按钮？
   - 是否加载了游戏？

5. **PPSSPP 设置**：
   - 音量设置
   - 按键音效是否启用

## 相关文档

- `ohos/AUDIO_TROUBLESHOOTING.md` - 详细的问题排查指南
- `ohos/AUDIO_TEST_TONE.md` - 测试音输出指南
- `ohos/AUDIO_DEBUG_GUIDE.md` - 调试指南
- `ohos/AUDIO_COMPLETE_SUMMARY.md` - 完整的实现总结

## 快速命令参考

```bash
# 重新编译
cd ohos && ./rebuild.sh

# 安装
hdc install entry/build/default/outputs/default/entry-default-signed.hap

# 清空日志
hdc shell hilog -r

# 监控音频日志
hdc shell hilog -x | grep -E "Audio|SetRenderCallback|Max sample"

# 监控所有 PPSSPP 日志
hdc shell hilog -x | grep PPSSPP

# 一键测试
cd ohos && ./quick_test.sh
```

