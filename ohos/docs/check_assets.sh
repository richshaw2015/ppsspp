#!/bin/bash
# 检查 assets 目录中的资源文件

echo "=== 检查 PPSSPP Assets 资源文件 ==="
echo ""

# 当前已配置的文件
echo "=== 当前已配置的文件 ==="
cat << 'EOF'
字体图集:
  - font_atlas.zim
  - font_atlas.meta
  - ppge_atlas.zim
  - ppge_atlas.meta
  - asciifont_atlas.zim
  - asciifont_atlas.meta

字体文件:
  - Roboto_Condensed-Regular.ttf
  - Roboto_Condensed-Bold.ttf
  - Roboto_Condensed-Light.ttf
  - Roboto_Condensed-Italic.ttf
  - Inconsolata-Regular.ttf

配置文件:
  - langregion.ini
  - compat.ini
  - compatvr.ini
  - knownfuncs.ini
  - gamecontrollerdb.txt

图标:
  - 7z.png
  - zip.png
  - unknown.png
  - rargray.png

UI 图片:
  - ui_images/bg.png
  - ui_images/drop_shadow.png
  - ui_images/icon_gold.png
  - ui_images/icon.png
  - ui_images/images.svg
  - ui_images/psp_display.png
  - ui_images/retroachievements_logo.png
  - ui_images/stick_line.png
  - ui_images/stick.png

音效文件:
  - sfx_back.wav
  - sfx_confirm.wav
  - sfx_select.wav
  - sfx_toggle_on.wav
  - sfx_toggle_off.wav
  - sfx_achievement_unlocked.wav
  - sfx_leaderbord_submitted.wav

语言文件:
  - lang/zh_CN.ini (简体中文)
  - lang/en_US.ini (英文)
EOF

echo ""
echo "=== 缺失的重要文件 ==="
echo ""

# 检查缺失的文件
missing_files=()

# 检查 infra-dns.json
if [ -f "assets/infra-dns.json" ]; then
    missing_files+=("infra-dns.json")
fi

# 检查 redump.csv
if [ -f "assets/redump.csv" ]; then
    missing_files+=("redump.csv")
fi

# 检查 ui_images/svg_sources.txt
if [ -f "assets/ui_images/svg_sources.txt" ]; then
    missing_files+=("ui_images/svg_sources.txt")
fi

if [ ${#missing_files[@]} -gt 0 ]; then
    echo "以下文件存在但未配置："
    for file in "${missing_files[@]}"; do
        echo "  - $file"
    done
else
    echo "没有发现缺失的重要文件"
fi

echo ""
echo "=== 未包含的目录（可选） ==="
echo ""
echo "以下目录未包含（通常不需要）："
echo "  - debugger/ (调试器 Web UI)"
echo "  - flash0/ (PSP 系统字体)"
echo "  - mime/ (MIME 类型定义)"
echo "  - shaders/ (着色器文件)"
echo "  - themes/ (主题文件)"
echo "  - upload/ (上传功能 HTML)"
echo "  - vfpu/ (VFPU 查找表)"
echo ""
echo "这些目录通常在运行时动态加载或不是必需的。"

echo ""
echo "=== 语言文件 ==="
echo ""
echo "当前只包含："
echo "  - zh_CN.ini (简体中文)"
echo "  - en_US.ini (英文)"
echo ""
echo "其他可用的语言文件："
ls -1 assets/lang/*.ini | grep -v "zh_CN\|en_US" | sed 's/assets\/lang\//  - lang\//'

echo ""
echo "=== 建议 ==="
echo ""
echo "1. 必需文件：当前配置已包含所有必需文件"
echo "2. 可选文件："
echo "   - infra-dns.json: DNS 配置（可选）"
echo "   - redump.csv: 游戏数据库（可选）"
echo "3. 语言文件：可以根据需要添加更多语言"
echo "4. 其他目录：通常不需要包含"

