#!/bin/bash
# 检查鸿蒙构建状态

echo "=== 鸿蒙 PPSSPP 构建检查 ==="
echo ""

# 检查关键文件
echo "1. 检查关键文件..."
files=(
    "entry/src/main/cpp/CMakeLists.txt"
    "entry/src/main/cpp/ohos_gl_context.cpp"
    "entry/src/main/cpp/ohos_gl_context.h"
    "entry/src/main/cpp/napi/napi_init.cpp"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file"
    else
        echo "  ✗ $file (缺失)"
    fi
done

echo ""
echo "2. 检查编译输出目录..."
if [ -d "entry/.cxx" ]; then
    echo "  ✓ 编译目录存在"
    echo "  构建配置:"
    ls -la entry/.cxx/default/default/ 2>/dev/null | grep "^d" | awk '{print "    - " $9}'
else
    echo "  ✗ 编译目录不存在（首次构建）"
fi

echo ""
echo "3. 最近的编译错误（如果有）..."
if [ -f "entry/.cxx/default/default/debug/arm64-v8a/build_output.txt" ]; then
    tail -20 entry/.cxx/default/default/debug/arm64-v8a/build_output.txt
else
    echo "  无编译日志"
fi

echo ""
echo "=== 检查完成 ==="
