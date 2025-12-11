# 音频测试音已启用

## 当前状态

已在 `ohos_audio_backend.cpp` 中添加测试音输出。

## 工作原理

当 PPSSPP 混音器没有产生音频数据时（Max sample < 0.001），会自动输出 440Hz 的测试音（A4 音符）。

```cpp
// 临时测试：如果 PPSSPP 没有音频，输出测试音
if (maxSample < 0.001f) {
    // 输出 440Hz 正弦波，10% 音量
    ...
}
```

## 测试步骤

### 1. 重新编译并安装

```bash
cd ohos
./rebuild.sh
hdc install entry/build/default/outputs/default/entry-default-signed.hap
```

### 2. 启动应用

在设备上启动 PPSSPP

### 3. 预期结果

**应该立即听到持续的 440Hz 音调**（类似电话拨号音）

## 结果分析

### 情况 A：能听到 440Hz 音调

✅ **音频输出路径正常！**

这说明：
- OHAudio 初始化成功
- 音频回调正常工作
- 音频数据能够正确输出到扬声器

**问题在于**：PPSSPP 混音器没有产生音频数据

**可能的原因**：
1. PPSSPP 音频系统配置问题
2. 按键音效未正确初始化
3. 需要特定的触发条件

### 情况 B：听不到任何声音

❌ **音频输出路径有问题**

可能的原因：
1. 设备音量为 0（检查设备音量）
2. OHAudio 配置问题
3. 音频权限问题
4. 音频路由问题（声音输出到其他设备）

## 下一步

### 如果能听到测试音

说明音频输出正常，需要解决 PPSSPP 混音器的问题：

1. **检查 PPSSPP 配置**：
   - 进入 设置 → 音频
   - 确认音量 > 0
   - 确认按键音效已启用

2. **检查 PPSSPP 音频初始化**：
   - 查看是否有 PPSSPP Core 的音频初始化日志
   - 确认 `NativeMix` 函数是否正常工作

3. **尝试加载游戏**：
   - 加载一个 PSP 游戏
   - 看是否有游戏音频输出

### 如果听不到测试音

需要检查音频输出路径：

1. **检查设备音量**：
   - 确保设备音量不是 0
   - 尝试调到最大

2. **检查音频路由**：
   - 确认声音输出到扬声器而不是蓝牙设备
   - 尝试插拔耳机

3. **检查日志**：
   ```bash
   hdc shell hilog -x | grep -E "AudioBackend|Test tone"
   ```
   
   应该看到：
   ```
   [PPSSPP_AudioBackend] Test tone output (PPSSPP silent)
   ```

## 移除测试音

测试完成后，如果要移除测试音，删除这段代码：

```cpp
// 临时测试：如果 PPSSPP 没有音频，输出测试音
if (maxSample < 0.001f) {
    ...
    return;
}
```

或者使用 git 恢复：

```bash
cd ohos/entry/src/main/cpp
git checkout ohos_audio_backend.cpp
```

## 日志监控

```bash
# 监控测试音日志
hdc shell hilog -x | grep "Test tone"

# 监控完整音频日志
hdc shell hilog -x | grep AudioBackend

# 监控 Max sample 值
hdc shell hilog -x | grep "Max sample"
```

## 预期日志

启动应用后，应该看到：

```
[PPSSPP_AudioBackend] AudioCallback #100: renderCallback_=0x..., numSamples=8192
[PPSSPP_AudioBackend] Max sample value: 0.0000 (numFrames=4096, rate=44100)
[PPSSPP_AudioBackend] Test tone output (PPSSPP silent)
```

并且能听到持续的 440Hz 音调。

## 总结

这个测试音可以帮助我们确定：
- ✅ 音频输出路径是否正常
- ✅ OHAudio 是否正确工作
- ✅ 音频回调是否被调用
- ❓ PPSSPP 混音器为什么没有产生音频

如果能听到测试音，说明我们的音频实现是正确的，只需要解决 PPSSPP 混音器的配置问题。

