# 更新 Assets 文件列表

## 快速更新

运行以下命令生成完整的文件列表：

```bash
find assets -type f \( \
  -name "*.zim" -o \
  -name "*.meta" -o \
  -name "*.ttf" -o \
  -name "*.ini" -o \
  -name "*.txt" -o \
  -name "*.png" -o \
  -name "*.svg" -o \
  -name "*.wav" \
\) | sed 's|^assets/||' | sort | awk '{print "      '\''" $0 "'\'',"}'
```

## 手动更新步骤

### 1. 生成文件列表

```bash
cd ppsspp
find assets -type f \( \
  -name "*.zim" -o \
  -name "*.meta" -o \
  -name "*.ttf" -o \
  -name "*.ini" -o \
  -name "*.txt" -o \
  -name "*.png" -o \
  -name "*.svg" -o \
  -name "*.wav" \
\) | sed 's|^assets/||' | sort | awk '{print "      '\''" $0 "'\'',"}'
```

### 2. 复制输出

将命令输出复制到剪贴板。

### 3. 更新 Index.ets

打开 `ohos/entry/src/main/ets/pages/Index.ets`

找到 `assetFiles` 数组定义：

```typescript
const assetFiles: string[] = [
  // 在这里替换所有内容
];
```

将复制的内容粘贴到数组中。

### 4. 验证

确保：
- 数组语法正确
- 最后一个元素后面有逗号
- 所有文件路径正确

## 当前必需文件（最小集合）

如果只想复制必需的文件，使用以下列表：

```typescript
const assetFiles: string[] = [
  // 字体图集（必需）
  'font_atlas.zim',
  'font_atlas.meta',
  'ppge_atlas.zim',
  'ppge_atlas.meta',
  'asciifont_atlas.zim',
  'asciifont_atlas.meta',
  
  // 字体文件（必需）
  'Roboto_Condensed-Regular.ttf',
  'Roboto_Condensed-Bold.ttf',
  'Roboto_Condensed-Light.ttf',
  'Roboto_Condensed-Italic.ttf',
  'Inconsolata-Regular.ttf',
  
  // 配置文件（必需）
  'langregion.ini',
  'compat.ini',
  'compatvr.ini',
  'knownfuncs.ini',
  'gamecontrollerdb.txt',
  
  // 图标（必需）
  '7z.png',
  'zip.png',
  'unknown.png',
  'rargray.png',
  
  // UI 图片（必需）
  'ui_images/bg.png',
  'ui_images/drop_shadow.png',
  'ui_images/icon_gold.png',
  'ui_images/icon.png',
  'ui_images/images.svg',
  'ui_images/psp_display.png',
  'ui_images/retroachievements_logo.png',
  'ui_images/stick_line.png',
  'ui_images/stick.png',
  
  // 音效文件（UI 音效必需）
  'sfx_back.wav',
  'sfx_confirm.wav',
  'sfx_select.wav',
  'sfx_toggle_on.wav',
  'sfx_toggle_off.wav',
  'sfx_achievement_unlocked.wav',
  'sfx_leaderbord_submitted.wav',
  
  // 语言文件（至少需要一个）
  'lang/zh_CN.ini',  // 简体中文
  'lang/en_US.ini',  // 英文
  
  // 可选：添加更多语言
  // 'lang/zh_TW.ini',  // 繁体中文
  // 'lang/ja_JP.ini',  // 日文
  // 'lang/ko_KR.ini',  // 韩文
];
```

## 完整文件列表（推荐）

包含所有语言和主题：

```bash
# 生成完整列表
find assets -type f \( \
  -name "*.zim" -o \
  -name "*.meta" -o \
  -name "*.ttf" -o \
  -name "*.ini" -o \
  -name "*.txt" -o \
  -name "*.png" -o \
  -name "*.svg" -o \
  -name "*.wav" \
\) | sed 's|^assets/||' | sort
```

共约 90 个文件，包括：
- 所有语言文件（54 个）
- 所有主题文件（6 个）
- 所有核心资源

## 测试

更新后，测试复制是否成功：

```bash
# 1. 卸载旧版本
hdc shell bm uninstall -n com.ppsspp.ppsspp

# 2. 重新编译
cd ohos && ./rebuild.sh

# 3. 安装
hdc install entry/build/default/outputs/default/entry-default-signed.hap

# 4. 查看复制日志
hdc shell hilog -x | grep "Copied:"

# 5. 统计复制的文件数
hdc shell hilog -x | grep "Copied:" | wc -l
```

应该看到所有文件都被复制。

## 故障排除

### 问题：某些文件复制失败

检查日志：
```bash
hdc shell hilog -x | grep "Failed to copy"
```

可能原因：
1. 文件不在 rawfile 中
2. 文件路径错误
3. 权限问题

### 问题：首次启动很慢

如果复制所有文件（90+ 个），首次启动可能需要几秒钟。这是正常的。

可以优化：
1. 只复制必需文件（~40 个）
2. 使用后台任务复制其他文件
3. 添加进度提示

## 总结

- **最小集合**：~40 个文件（核心功能）
- **推荐集合**：~90 个文件（完整功能）
- **更新方法**：使用脚本生成，手动复制到 Index.ets
- **测试方法**：卸载旧版本，重新安装，查看日志

