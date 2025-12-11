#!/bin/bash

# 检查 PPSSPP 鸿蒙库的大小和符号

SO_PATH="entry/build/default/intermediates/cmake/default/obj/arm64-v8a/libppsspp_ohos.so"

if [ ! -f "$SO_PATH" ]; then
    echo "❌ 找不到 libppsspp_ohos.so"
    echo "请先编译项目"
    exit 1
fi

echo "=== PPSSPP 鸿蒙库信息 ==="
echo ""

echo "📦 文件大小:"
ls -lh "$SO_PATH" | awk '{print "  " $5 " - " $9}'
echo ""

echo "🔍 符号统计:"
TOTAL_SYMBOLS=$(nm -D "$SO_PATH" 2>/dev/null | wc -l)
DEFINED_SYMBOLS=$(nm -D "$SO_PATH" 2>/dev/null | grep -v " U " | wc -l)
UNDEFINED_SYMBOLS=$(nm -D "$SO_PATH" 2>/dev/null | grep " U " | wc -l)

echo "  总符号数: $TOTAL_SYMBOLS"
echo "  已定义符号: $DEFINED_SYMBOLS"
echo "  未定义符号: $UNDEFINED_SYMBOLS"
echo ""

echo "🔎 检查关键符号:"
check_symbol() {
    local pattern=$1
    local name=$2
    local count=$(nm -D "$SO_PATH" 2>/dev/null | grep "$pattern" | wc -l)
    if [ $count -gt 0 ]; then
        echo "  ✅ $name: $count 个符号"
    else
        echo "  ❌ $name: 未找到"
    fi
}

check_symbol "Common" "Common 库"
check_symbol "Core" "Core 库"
check_symbol "GPU" "GPU 模块"
check_symbol "MIPS" "MIPS 模拟器"
check_symbol "HLE" "HLE 系统"

echo ""
echo "📊 静态库大小:"
find ohos/entry/.cxx -name "libCommon.a" -o -name "libCore.a" -o -name "libnative.a" 2>/dev/null | while read lib; do
    ls -lh "$lib" | awk '{print "  " $9 ": " $5}'
done

echo ""
echo "💡 预期大小: 15-30MB (Debug), 5-10MB (Release)"
