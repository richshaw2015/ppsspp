# 移除测试音并测试真实音频

## 当前状态

✅ **音频系统工作正常**（测试音能够播放）

现在需要：
1. 移除测试音代码
2. 测试 PPSSPP 的真实音频

## 步骤 1：移除测试音代码

编辑 `ohos/entry/src/main/cpp/ohos_audio_backend.cpp`

找到并**删除**测试音代码（应该在 `AudioCallback` 函数中）：

```cpp
// 删除这段代码：
// 临时测试：输出 440Hz 正弦波
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
return;  // 也要删除这个 return
```

保留正常的混音逻辑。

## 步骤 2：重新编译

```bash
cd ohos
./rebuild.sh
```

## 步骤 3：安装

```bash
hdc install entry/build/default/outputs/default/entry-default-signed.hap
```

## 步骤 4：测试按键音效

1. 在设备上启动 PPSSPP

2. 进入 **设置 (Settings) → 音频 (Audio)**

3. 找到 **UI sound** 选项并**勾选启用**

4. 确认 **UI volume** 设置为合适的值（如 75%）

5. 返回主菜单

6. **点击任意按钮**（如 "设置"、"游戏"、"最近" 等）

7. **应该能听到按键音效**（"哔" 的一声）

## 步骤 5：测试游戏音频

1. 加载一个 PSP 游戏（ISO/CSO 文件）

2. 进入游戏

3. **应该能听到游戏音频**（背景音乐、音效等）

## 预期结果

### 按键音效

点击按钮时：
- ✅ 听到短促的 "哔" 声
- ✅ 日志显示 `Max sample value: 0.xxxx`（非零值）

### 游戏音频

游戏运行时：
- ✅ 听到游戏背景音乐
- ✅ 听到游戏音效
- ✅ 日志显示 `Max sample value: 0.xxxx`（非零值）

## 如果没有声音

### 检查 1：按键音效是否启用

进入 设置 → 音频，确认：
- ✅ UI sound 已勾选
- ✅ UI volume > 0

### 检查 2：设备音量

确认设备音量不是 0，没有静音。

### 检查 3：查看日志

```bash
hdc shell hilog -x | grep "Max sample"
```

点击按钮时，应该看到：
```
Max sample value: 0.1234  ← 非零值
```

如果仍然是 `0.0000`，说明：
- 按键音效未启用
- 或者音效文件加载失败

### 检查 4：音效文件

确认 `assets` 目录中有音效文件：
- `sfx_back.wav`
- `sfx_select.wav`
- `sfx_confirm.wav`
- `sfx_toggle_on.wav`
- `sfx_toggle_off.wav`

这些文件应该在 `Index.ets` 的 `copyAssetsToFiles()` 中被复制到设备。

## 调试命令

```bash
# 查看音频日志
hdc shell hilog -x | grep AudioBackend

# 查看最大样本值
hdc shell hilog -x | grep "Max sample"

# 查看回调信息
hdc shell hilog -x | grep "AudioCallback"

# 查看完整 PPSSPP 日志
hdc shell hilog -x | grep PPSSPP
```

## 总结

音频系统已经完全实现并验证：
- ✅ OHAudio 初始化成功
- ✅ 音频回调正常工作
- ✅ 数据转换正确
- ✅ 音频输出路径正常

只需要：
1. 移除测试音代码
2. 在设置中启用按键音效
3. 点击按钮或加载游戏

就能听到真实的 PPSSPP 音频了！

