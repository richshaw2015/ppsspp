# 推荐的 Assets 配置

## 当前配置（最小）

当前 `Index.ets` 中的配置包含 42 个文件，满足基本需求。

## 推荐配置（添加常用语言）

建议添加以下语言文件，以支持更多用户：

```typescript
const assetFiles: string[] = [
  // ... 现有文件 ...
  
  // 语言文件 - 简体中文
  'lang/zh_CN.ini',
  // 语言文件 - 繁体中文
  'lang/zh_TW.ini',
  // 语言文件 - 英文
  'lang/en_US.ini',
  // 语言文件 - 日语（PSP 主要市场）
  'lang/ja_JP.ini',
  // 语言文件 - 韩语（PSP 主要市场）
  'lang/ko_KR.ini',
];
```

## 完整配置示例

如果需要支持更多语言和功能：

```typescript
const assetFiles: string[] = [
  // 字体图集
  'font_atlas.zim',
  'font_atlas.meta',
  'ppge_atlas.zim',
  'ppge_atlas.meta',
  'asciifont_atlas.zim',
  'asciifont_atlas.meta',
  
  // 字体文件
  'Roboto_Condensed-Regular.ttf',
  'Roboto_Condensed-Bold.ttf',
  'Roboto_Condensed-Light.ttf',
  'Roboto_Condensed-Italic.ttf',
  'Inconsolata-Regular.ttf',
  
  // 配置文件
  'langregion.ini',
  'compat.ini',
  'compatvr.ini',
  'knownfuncs.ini',
  'gamecontrollerdb.txt',
  'infra-dns.json',  // 新增：DNS 配置
  
  // 图标
  '7z.png',
  'zip.png',
  'unknown.png',
  'rargray.png',
  
  // UI 图片
  'ui_images/bg.png',
  'ui_images/drop_shadow.png',
  'ui_images/icon_gold.png',
  'ui_images/icon.png',
  'ui_images/images.svg',
  'ui_images/psp_display.png',
  'ui_images/retroachievements_logo.png',
  'ui_images/stick_line.png',
  'ui_images/stick.png',
  
  // 音效文件
  'sfx_back.wav',
  'sfx_confirm.wav',
  'sfx_select.wav',
  'sfx_toggle_on.wav',
  'sfx_toggle_off.wav',
  'sfx_achievement_unlocked.wav',
  'sfx_leaderbord_submitted.wav',
  
  // 语言文件 - 中文
  'lang/zh_CN.ini',  // 简体中文
  'lang/zh_TW.ini',  // 繁体中文
  
  // 语言文件 - 英文
  'lang/en_US.ini',
  
  // 语言文件 - 亚洲语言
  'lang/ja_JP.ini',  // 日语
  'lang/ko_KR.ini',  // 韩语
  'lang/th_TH.ini',  // 泰语
  'lang/vi_VN.ini',  // 越南语
  'lang/id_ID.ini',  // 印尼语
  'lang/ms_MY.ini',  // 马来语
  
  // 语言文件 - 欧洲语言
  'lang/de_DE.ini',  // 德语
  'lang/fr_FR.ini',  // 法语
  'lang/es_ES.ini',  // 西班牙语
  'lang/it_IT.ini',  // 意大利语
  'lang/pt_BR.ini',  // 巴西葡萄牙语
  'lang/ru_RU.ini',  // 俄语
  'lang/pl_PL.ini',  // 波兰语
  'lang/nl_NL.ini',  // 荷兰语
  'lang/tr_TR.ini',  // 土耳其语
];
```

## 如何更新配置

### 方法 1：手动编辑

编辑 `ohos/entry/src/main/ets/pages/Index.ets`，在 `assetFiles` 数组中添加需要的文件。

### 方法 2：使用脚本生成

创建一个脚本来自动生成配置：

```bash
#!/bin/bash
# generate_asset_config.sh

echo "const assetFiles: string[] = ["

# 字体图集
echo "  // 字体图集"
for file in font_atlas.zim font_atlas.meta ppge_atlas.zim ppge_atlas.meta asciifont_atlas.zim asciifont_atlas.meta; do
    echo "  '$file',"
done

# 字体文件
echo "  // 字体文件"
for file in Roboto_Condensed-*.ttf Inconsolata-Regular.ttf; do
    echo "  '$file',"
done

# ... 其他文件 ...

# 语言文件
echo "  // 语言文件"
for lang in zh_CN zh_TW en_US ja_JP ko_KR; do
    echo "  'lang/${lang}.ini',"
done

echo "];"
```

## 建议

### 最小配置（当前）
- ✅ 适合：快速开发和测试
- ✅ 大小：约 7-8 MB
- ✅ 语言：中文、英文

### 推荐配置
- ✅ 适合：正式发布
- ✅ 大小：约 8-9 MB
- ✅ 语言：中文（简繁）、英文、日语、韩语
- ✅ 额外功能：DNS 配置

### 完整配置
- ✅ 适合：国际版本
- ✅ 大小：约 10-12 MB
- ✅ 语言：20+ 种语言
- ✅ 额外功能：完整的语言支持

## 注意事项

1. **文件大小**：每个语言文件约 50 KB，添加太多会增加应用大小
2. **首次启动**：文件越多，首次复制时间越长
3. **存储空间**：确保设备有足够的存储空间
4. **按需加载**：考虑实现按需下载语言包的功能

## 当前状态

✅ **当前配置完全满足需求**

- 包含所有核心文件
- 支持中英文
- 应用可以正常运行

如果需要支持更多语言，可以参考上述配置逐步添加。

