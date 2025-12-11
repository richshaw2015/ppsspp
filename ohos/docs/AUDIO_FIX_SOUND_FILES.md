# 音频问题修复：添加音效文件

## 问题原因

**音效文件没有被复制到设备！**

在 `Index.ets` 的 `assetFiles` 数组中，缺少音效文件：
- `sfx_back.wav`
- `sfx_confirm.wav`
- `sfx_select.wav`
- `sfx_toggle_on.wav`
- `sfx_toggle_off.wav`
- `sfx_achievement_unlocked.wav`
- `sfx_leaderbord_submitted.wav`

虽然这些文件在 `ohos/entry/src/main/resources/rawfile/assets/` 中存在，但没有被添加到复制列表，所以不会被复制到设备的 `context.filesDir/assets/` 目录。

## 修复内容

已在 `Index.ets` 中添加音效文件到 `assetFiles` 数组：

```typescript
// 音效文件
'sfx_back.wav',
'sfx_confirm.wav',
'sfx_select.wav',
'sfx_toggle_on.wav',
'sfx_toggle_off.wav',
'sfx_achievement_unlocked.wav',
'sfx_leaderbord_submitted.wav',
```

## 测试步骤

### 1. 清除应用数据（重要！）

由于之前的版本没有复制音效文件，需要清除应用数据以触发重新复制：

```bash
# 卸载应用
hdc shell bm uninstall -n com.ppsspp.ppsspp

# 或者清除应用数据
hdc shell rm -rf /data/app/el2/100/base/com.ppsspp.ppsspp
```

### 2. 重新编译并安装

```bash
cd ohos
./rebuild.sh
hdc install entry/build/default/outputs/default/entry-default-signed.hap
```

### 3. 启动应用

在设备上启动 PPSSPP。

应用会自动复制所有资源文件，包括音效文件。

### 4. 检查音效文件是否复制成功

```bash
# 查看应用日志
hdc shell hilog -x | grep "Copied.*sfx"
```

应该看到：
```
[PPSSPP] Copied: sfx_back.wav
[PPSSPP] Copied: sfx_confirm.wav
[PPSSPP] Copied: sfx_select.wav
[PPSSPP] Copied: sfx_toggle_on.wav
[PPSSPP] Copied: sfx_toggle_off.wav
[PPSSPP] Copied: sfx_achievement_unlocked.wav
[PPSSPP] Copied: sfx_leaderbord_submitted.wav
```

### 5. 启用按键音效

1. 进入 **设置 → 音频**
2. 勾选 **UI sound**
3. 确认 **UI volume** > 0（如 75%）
4. 返回主菜单

### 6. 测试按键音效

点击任意按钮（如 "设置"、"游戏" 等）

**应该能听到按键音效！**

### 7. 查看日志

```bash
hdc shell hilog -x | grep "Max sample"
```

点击按钮时，应该看到：
```
Max sample value: 0.1234  ← 非零值！
```

## 为什么之前测试音能播放但按键音效不行？

- **测试音**：直接在音频回调中生成，不需要文件
- **按键音效**：需要从 WAV 文件加载，文件不存在就无法播放

所以测试音能播放证明音频输出路径正常，但按键音效需要文件支持。

## 预期结果

### 点击按钮前

```
Max sample value: 0.0000
```

### 点击按钮后

```
Max sample value: 0.1234  ← 非零值
```

并且能听到短促的 "哔" 声。

## 如果仍然没有声音

### 检查 1：音效文件是否复制成功

```bash
# 查看日志
hdc shell hilog -x | grep "Copied.*sfx"
```

如果没有看到 "Copied: sfx_*.wav"，说明文件复制失败。

### 检查 2：音效文件是否存在

由于 OHOS 的安全限制，无法直接查看应用私有目录。但可以通过日志确认。

### 检查 3：音效加载日志

查看是否有音效加载相关的日志：

```bash
hdc shell hilog -x | grep -E "Sample|SFX|Sound"
```

### 检查 4：UI sound 是否启用

确认在设置中：
- ✅ UI sound 已勾选
- ✅ UI volume > 0

## 移除测试音

现在音效文件已经添加，可以移除测试音代码了。

在 `ohos/entry/src/main/cpp/ohos_audio_backend.cpp` 中，删除测试音代码：

```cpp
// 删除这段：
// 临时测试：如果 PPSSPP 没有音频，输出测试音
if (maxSample < 0.001f) {
    ...
    return;
}
```

然后重新编译。

## 总结

问题的根本原因：
- ❌ 音效文件没有被复制到设备
- ✅ 音频系统本身是正常的（测试音能播放）

修复方法：
- ✅ 添加音效文件到 `assetFiles` 数组
- ✅ 清除应用数据并重新安装
- ✅ 启用 UI sound
- ✅ 点击按钮测试

现在应该能听到按键音效了！

