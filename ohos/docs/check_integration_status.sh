#!/bin/bash
# 检查 PPSSPP 鸿蒙适配集成状态

echo "=========================================="
echo "PPSSPP 鸿蒙适配 - 集成状态检查"
echo "=========================================="
echo ""

# 定义颜色
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 统计函数
count_lines() {
    if [ -f "$1" ]; then
        wc -l < "$1" | tr -d ' '
    else
        echo "0"
    fi
}

# 1. 检查第三方依赖库
echo -e "${BLUE}1. 第三方依赖库状态${NC}"
echo "----------------------------------------"

EXT_LIBS=(
    "cpu_features" "snappy" "zlib" "libpng17" "libzip"
    "glslang" "SPIRV-Cross" "minimp3" "udis86" "at3_standalone"
    "libchdr" "rcheevos" "lua" "gason" "cityhash"
    "vma" "kirk" "sfmt19937" "xbrz" "xxhash"
    "basis_universal" "armips"
)

FOUND=0
TOTAL=${#EXT_LIBS[@]}

for lib in "${EXT_LIBS[@]}"; do
    if [ -d "../ext/$lib" ] && [ "$(ls -A ../ext/$lib)" ]; then
        FOUND=$((FOUND + 1))
    fi
done

echo -e "已找到: ${GREEN}$FOUND${NC} / $TOTAL 个依赖库"
echo ""

# 2. 检查源文件列表
echo -e "${BLUE}2. 源文件列表状态${NC}"
echo "----------------------------------------"

COMMON_SOURCES="entry/src/main/cpp/common_sources.cmake"
CORE_SOURCES="entry/src/main/cpp/core_sources.cmake"

COMMON_LINES=$(count_lines "$COMMON_SOURCES")
CORE_LINES=$(count_lines "$CORE_SOURCES")

echo "Common 源文件列表: $COMMON_LINES 行"
if [ "$COMMON_LINES" -lt 50 ]; then
    echo -e "  ${YELLOW}⚠ 需要完善 (预期 200+ 行)${NC}"
else
    echo -e "  ${GREEN}✓ 已完善${NC}"
fi

echo "Core 源文件列表: $CORE_LINES 行"
if [ "$CORE_LINES" -lt 100 ]; then
    echo -e "  ${YELLOW}⚠ 需要完善 (预期 1000+ 行)${NC}"
else
    echo -e "  ${GREEN}✓ 已完善${NC}"
fi
echo ""

# 3. 检查平台适配文件
echo -e "${BLUE}3. 平台适配文件状态${NC}"
echo "----------------------------------------"

OHOS_FILES=(
    "entry/src/main/cpp/ohos_app.cpp"
    "entry/src/main/cpp/ohos_system.cpp"
    "entry/src/main/cpp/ohos_audio.cpp"
    "entry/src/main/cpp/ohos_gl_context.cpp"
    "entry/src/main/cpp/ohos_input.cpp"
)

OHOS_FOUND=0
OHOS_TOTAL=${#OHOS_FILES[@]}

for file in "${OHOS_FILES[@]}"; do
    if [ -f "$file" ]; then
        OHOS_FOUND=$((OHOS_FOUND + 1))
        echo -e "${GREEN}✓${NC} $(basename $file)"
    else
        echo -e "${RED}✗${NC} $(basename $file) - 缺失"
    fi
done

echo ""
echo "平台适配文件: $OHOS_FOUND / $OHOS_TOTAL"
echo ""

# 4. 检查构建配置
echo -e "${BLUE}4. 构建配置状态${NC}"
echo "----------------------------------------"

CMAKE_FILE="entry/src/main/cpp/CMakeLists.txt"
CMAKE_LINES=$(count_lines "$CMAKE_FILE")

echo "CMakeLists.txt: $CMAKE_LINES 行"

# 检查关键配置
if grep -q "add_library(Common STATIC" "$CMAKE_FILE" 2>/dev/null; then
    echo -e "${GREEN}✓${NC} Common 库已配置"
else
    echo -e "${YELLOW}⚠${NC} Common 库未配置"
fi

if grep -q "add_library(Core STATIC" "$CMAKE_FILE" 2>/dev/null; then
    echo -e "${GREEN}✓${NC} Core 库已配置"
else
    echo -e "${YELLOW}⚠${NC} Core 库未配置"
fi

if grep -q "add_library(native STATIC" "$CMAKE_FILE" 2>/dev/null; then
    echo -e "${GREEN}✓${NC} native 库已配置"
else
    echo -e "${YELLOW}⚠${NC} native 库未配置"
fi

echo ""

# 5. 检查文档
echo -e "${BLUE}5. 文档状态${NC}"
echo "----------------------------------------"

DOCS=(
    "docs/CORE_INTEGRATION_PLAN.md"
    "docs/DEPENDENCIES_INTEGRATION.md"
    "docs/INTEGRATION_SUMMARY.md"
    "QUICK_START.md"
)

DOC_FOUND=0
DOC_TOTAL=${#DOCS[@]}

for doc in "${DOCS[@]}"; do
    if [ -f "$doc" ]; then
        DOC_FOUND=$((DOC_FOUND + 1))
        echo -e "${GREEN}✓${NC} $(basename $doc)"
    else
        echo -e "${RED}✗${NC} $(basename $doc) - 缺失"
    fi
done

echo ""
echo "文档完成度: $DOC_FOUND / $DOC_TOTAL"
echo ""

# 6. 总体进度
echo "=========================================="
echo -e "${BLUE}总体进度${NC}"
echo "=========================================="
echo ""

# 计算总体完成度
TOTAL_TASKS=5
COMPLETED_TASKS=0

# 任务 1: 第三方依赖库
if [ "$FOUND" -eq "$TOTAL" ]; then
    COMPLETED_TASKS=$((COMPLETED_TASKS + 1))
    echo -e "${GREEN}✓${NC} 第三方依赖库集成 (100%)"
else
    PERCENT=$((FOUND * 100 / TOTAL))
    echo -e "${YELLOW}⏳${NC} 第三方依赖库集成 ($PERCENT%)"
fi

# 任务 2: Common 源文件
if [ "$COMMON_LINES" -ge 50 ]; then
    COMPLETED_TASKS=$((COMPLETED_TASKS + 1))
    echo -e "${GREEN}✓${NC} Common 源文件列表"
else
    echo -e "${YELLOW}⏳${NC} Common 源文件列表 (需要完善)"
fi

# 任务 3: Core 源文件
if [ "$CORE_LINES" -ge 100 ]; then
    COMPLETED_TASKS=$((COMPLETED_TASKS + 1))
    echo -e "${GREEN}✓${NC} Core 源文件列表"
else
    echo -e "${YELLOW}⏳${NC} Core 源文件列表 (需要完善)"
fi

# 任务 4: 平台适配
if [ "$OHOS_FOUND" -eq "$OHOS_TOTAL" ]; then
    COMPLETED_TASKS=$((COMPLETED_TASKS + 1))
    echo -e "${GREEN}✓${NC} 平台适配文件"
else
    echo -e "${YELLOW}⏳${NC} 平台适配文件 ($OHOS_FOUND/$OHOS_TOTAL)"
fi

# 任务 5: 文档
if [ "$DOC_FOUND" -eq "$DOC_TOTAL" ]; then
    COMPLETED_TASKS=$((COMPLETED_TASKS + 1))
    echo -e "${GREEN}✓${NC} 文档"
else
    echo -e "${YELLOW}⏳${NC} 文档 ($DOC_FOUND/$DOC_TOTAL)"
fi

echo ""
PROGRESS=$((COMPLETED_TASKS * 100 / TOTAL_TASKS))
echo -e "总体完成度: ${GREEN}$PROGRESS%${NC} ($COMPLETED_TASKS/$TOTAL_TASKS)"

echo ""
echo "=========================================="
echo -e "${BLUE}下一步操作建议${NC}"
echo "=========================================="
echo ""

if [ "$COMMON_LINES" -lt 50 ]; then
    echo "1. 完善 Common 源文件列表"
    echo "   编辑: entry/src/main/cpp/common_sources.cmake"
    echo "   参考: ../CMakeLists.txt (搜索 'add_library(Common STATIC')"
    echo ""
fi

if [ "$CORE_LINES" -lt 100 ]; then
    echo "2. 完善 Core 源文件列表"
    echo "   编辑: entry/src/main/cpp/core_sources.cmake"
    echo "   参考: ../CMakeLists.txt (搜索 'add_library(\${CoreLibName}')"
    echo ""
fi

if [ "$OHOS_FOUND" -lt "$OHOS_TOTAL" ]; then
    echo "3. 创建缺失的平台适配文件"
    echo ""
fi

if [ "$PROGRESS" -ge 60 ]; then
    echo "4. 尝试编译"
    echo "   运行: ./hvigorw assembleHap"
    echo ""
fi

echo "查看详细指南: cat QUICK_START.md"
echo ""
