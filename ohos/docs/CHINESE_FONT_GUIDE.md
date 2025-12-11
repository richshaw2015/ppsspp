# PPSSPP 中文字体替换指南

## 概述

PPSSPP 项目使用多层字体系统来处理不同场景的文本渲染：
- **PSP 游戏字体**: 使用 PGF 格式，位于 `assets/flash0/font/`
- **UI 界面字体**: 使用字体图集技术，通过 TTF 字体生成
- **文本渲染**: 支持多种字体格式和语言

## 当前中文字体支持

### PSP 游戏中文字体
- `zh_gb.pgf`: 专为中文翻译游戏添加的字体
- 位置: `assets/flash0/font/zh_gb.pgf`
- 字体名称: "FTT-NewRodin Pro DB"

### UI 界面中文字体
- 主字体: `Roboto_Condensed-Regular.ttf`
- 中文备用字体: `ARIALUNI.ttf` (通过 `font_atlasscript.txt` 配置)

## 替换中文字体的方法

### 方法一: 使用自动化脚本 (推荐)

```bash
# 使用提供的脚本
./replace_chinese_font.sh /path/to/your/chinese-font.ttf
```

### 方法二: 手动替换

#### 步骤 1: 准备字体文件
将支持中文的 TTF 字体文件复制到 `assets/` 目录：

```bash
cp your-chinese-font.ttf assets/chinese-font.ttf
```

#### 步骤 2: 修改字体图集配置
编辑 `font_atlasscript.txt`:

```
2048
font UBUNTU24 assets/Roboto_Condensed-Regular.ttf UWERS 34 -2
font UBUNTU24 assets/chinese-font.ttf UWEhkcRGHKVTeS一二三四五六七八九十中文测试 28 0
```

参数说明:
- `2048`: 图集大小
- `UBUNTU24`: 字体标识符
- `assets/chinese-font.ttf`: 字体文件路径
- `UWEhkcRGHKVTeS一二三四五六七八九十中文测试`: 要包含的字符集
- `28`: 字体大小
- `0`: Y 偏移

#### 步骤 3: 重新生成字体图集
```bash
./build_fontatlas.sh
```

#### 步骤 4: 重新编译项目
```bash
# 根据你的构建系统
make clean && make
# 或
cmake --build build --clean-first
```

### 方法三: 添加新的中文字体条目

如果需要添加更多中文字体支持，可以修改 `Core/HLE/sceFont.cpp` 中的字体注册表：

```cpp
static const FontRegistryEntry fontRegistry[] = {
    // 现有条目...
    
    // 添加新的中文字体条目
    { 0x288, 0x288, 0x2000, 0x2000, 0, 0, FONT_FAMILY_SANS_SERIF, FONT_STYLE_REGULAR, 0, FONT_LANGUAGE_CHINESE, 0, 1, "zh_cn_new.pgf", "Your Chinese Font Name", 0, 0, 1581700, 145844, false },
};
```

## 推荐的中文字体

### 开源字体 (推荐)
1. **思源黑体 (Noto Sans CJK)**
   - 支持简体中文、繁体中文、日文、韩文
   - Google 和 Adobe 联合开发
   - 下载: https://github.com/googlefonts/noto-cjk

2. **文泉驿微米黑 (WenQuanYi Micro Hei)**
   - 专为中文优化
   - 开源免费
   - 适合屏幕显示

3. **思源宋体 (Noto Serif CJK)**
   - 宋体风格，适合正文阅读
   - 支持多种中文变体

### 商业字体
1. **微软雅黑 (Microsoft YaHei)**
   - Windows 系统默认中文字体
   - 显示效果优秀

2. **华文黑体 (STHeiti)**
   - macOS 系统中文字体
   - 适合界面显示

## 字体配置参数详解

### 字体图集脚本参数
```
font [标识符] [字体文件路径] [字符集] [大小] [Y偏移]
```

- **标识符**: 字体的内部标识符
- **字体文件路径**: 相对于项目根目录的路径
- **字符集**: 要包含在图集中的字符
- **大小**: 字体渲染大小 (像素)
- **Y偏移**: 垂直偏移调整

### 常用中文字符集
```
# 基本中文字符
一二三四五六七八九十中文测试字体显示效果

# 扩展中文字符集 (包含更多常用汉字)
的一是在不了有和人这中大为上个国我以要他时来用们生到作地于出就分对成会可主发年动同工也能下过子说产种面而方后多定行学法所民得经十三之进着等部度家电力里如水化高自二理起小物现实加量都两体制机当使点从业本去把性好应开它合还因由其些然前外天政四日那社义事平形相全表间样与关各重新线内数正心反你明看原又么利比或但质气第向道命此变条只没结解问意建月公无系军很情者最立代想已通并提直题党程展五果料象员革位入常文总次品式活设及管特件长求老头基资边流路级少图山统接知较将组见计别她手角期根论运农指几九区强放决西被干做必战先回则任取据处队南给色光门即保治北造百规热领七海口东导器压志世金增争济阶油思术极交受联什认六共权收证改清己美再采转更单风切打白教速花带安场身车例真务具万每目至达走积示议声报斗完类八离华名确才科张信马节话米整空元况今集温传土许步群广石记需段研界拉林律叫且究观越织装影算低持音众书布复容儿须际商非验连断深难近矿千周委素技备半办青省列习响约支般史感劳便团往酸历市克何除消构府称太准精值号率族维划选标写存候毛亲快效斯院查江型眼王按格养易置派层片始却专状育厂京识适属圆包火住调满县局照参红细引听该铁价严龙飞
```

## 故障排除

### 常见问题

1. **字体显示为方块或问号**
   - 检查字体文件是否包含所需字符
   - 确认字体图集是否正确生成
   - 验证字符集配置是否包含相应字符

2. **编译错误**
   - 确保字体文件路径正确
   - 检查字体图集生成工具是否可用
   - 验证字体文件格式是否支持

3. **性能问题**
   - 减少字体图集中的字符数量
   - 调整字体大小参数
   - 考虑使用多个小图集而非单个大图集

### 调试方法

1. **检查字体图集生成**
   ```bash
   # 查看生成的图集文件
   ls -la assets/font_atlas.*
   ```

2. **验证字符包含**
   - 检查 `font_atlasscript.txt` 中的字符集配置
   - 确认所需字符都在配置的字符集中

3. **测试字体渲染**
   - 在游戏中测试中文显示效果
   - 检查 UI 界面中文显示

## 高级配置

### 多字体支持
可以配置多个字体来支持不同的字符集：

```
2048
font LATIN assets/Roboto_Condensed-Regular.ttf ABCDEFGHIJKLMNOPQRSTUVWXYZ 34 -2
font CHINESE assets/chinese-font.ttf 一二三四五六七八九十中文 28 0
font JAPANESE assets/japanese-font.ttf ひらがなカタカナ漢字 28 0
```

### 字体回退机制
系统会按照配置顺序查找字符，实现字体回退：
1. 首先在主字体中查找
2. 如果未找到，在备用字体中查找
3. 最后使用默认替换字符

## 贡献指南

如果你改进了中文字体支持，欢迎提交 Pull Request：

1. Fork 项目
2. 创建功能分支
3. 提交更改
4. 创建 Pull Request

请确保：
- 测试了中文显示效果
- 更新了相关文档
- 遵循项目的代码风格