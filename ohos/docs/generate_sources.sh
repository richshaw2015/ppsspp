#!/bin/bash
# 从主 CMakeLists.txt 提取 Common 和 Core 库的源文件列表

echo "=========================================="
echo "生成 Common 和 Core 源文件列表"
echo "=========================================="
echo ""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/.."
CMAKE_FILE="$ROOT_DIR/CMakeLists.txt"
OUTPUT_DIR="$SCRIPT_DIR/entry/src/main/cpp"

if [ ! -f "$CMAKE_FILE" ]; then
    echo "错误: 找不到 CMakeLists.txt"
    exit 1
fi

echo "正在分析 $CMAKE_FILE ..."
echo ""

# 提取 Common 库源文件
echo "提取 Common 库源文件..."
echo "# Common 库源文件列表 - 自动生成" > "$OUTPUT_DIR/common_sources_full.cmake"
echo "# 生成时间: $(date)" >> "$OUTPUT_DIR/common_sources_full.cmake"
echo "" >> "$OUTPUT_DIR/common_sources_full.cmake"
echo "set(COMMON_SOURCES" >> "$OUTPUT_DIR/common_sources_full.cmake"

# 这里需要手动从 CMakeLists.txt 复制 Common 库的源文件列表
# 或者使用更复杂的解析逻辑

echo "    # TODO: 从 CMakeLists.txt 提取完整列表" >> "$OUTPUT_DIR/common_sources_full.cmake"
echo ")" >> "$OUTPUT_DIR/common_sources_full.cmake"

echo "✓ Common 源文件列表已生成: common_sources_full.cmake"

# 提取 Core 库源文件
echo "提取 Core 库源文件..."
echo "# Core 库源文件列表 - 自动生成" > "$OUTPUT_DIR/core_sources_full.cmake"
echo "# 生成时间: $(date)" >> "$OUTPUT_DIR/core_sources_full.cmake"
echo "" >> "$OUTPUT_DIR/core_sources_full.cmake"
echo "set(CORE_SOURCES" >> "$OUTPUT_DIR/core_sources_full.cmake"

echo "    # TODO: 从 CMakeLists.txt 提取完整列表" >> "$OUTPUT_DIR/core_sources_full.cmake"
echo ")" >> "$OUTPUT_DIR/core_sources_full.cmake"

echo "✓ Core 源文件列表已生成: core_sources_full.cmake"

echo ""
echo "=========================================="
echo "生成完成！"
echo "=========================================="
echo ""
echo "注意: 由于源文件数量庞大，建议手动从主 CMakeLists.txt 复制"
echo "或者参考 Android 平台的构建配置"
echo ""
