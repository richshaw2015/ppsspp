#!/bin/bash

# OHOS Rawfile 实现测试脚本

echo "=== OHOS Rawfile 实现测试 ==="

# 检查编译状态
echo "1. 检查编译状态..."
cd "$(dirname "$0")"

# 编译项目
echo "正在编译项目..."
./rebuild.sh > build.log 2>&1

# 检查编译结果
if grep -q "error:" build.log; then
    echo "❌ 编译失败，错误信息："
    grep -A 3 "error:" build.log | head -20
    echo ""
    echo "完整日志请查看 build.log"
    exit 1
else
    echo "✅ 编译成功"
fi

# 检查关键文件
echo ""
echo "2. 检查关键文件..."

FILES=(
    "entry/src/main/cpp/ohos_rawfile_reader.h"
    "entry/src/main/cpp/ohos_rawfile_reader.cpp"
    "entry/src/main/cpp/napi/napi_init.cpp"
    "entry/src/main/ets/pages/Index.ets"
)

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✅ $file 存在"
    else
        echo "❌ $file 不存在"
    fi
done

# 检查实现状态
echo ""
echo "3. 检查实现状态..."

# 检查 VFS 接口实现
if grep -q "class OhosRawfileReader : public VFSBackend" entry/src/main/cpp/ohos_rawfile_reader.h; then
    echo "✅ VFSBackend 接口实现"
else
    echo "❌ VFSBackend 接口未实现"
fi

# 检查函数签名
if grep -q "GetFileListing(const char \*path" entry/src/main/cpp/ohos_rawfile_reader.h; then
    echo "✅ GetFileListing 函数签名正确"
else
    echo "❌ GetFileListing 函数签名错误"
fi

if grep -q "uint8_t \*ReadFile" entry/src/main/cpp/ohos_rawfile_reader.h; then
    echo "✅ ReadFile 函数签名正确"
else
    echo "❌ ReadFile 函数签名错误"
fi

# 检查 NAPI 集成
if grep -q "ResourceManager" entry/src/main/cpp/napi/napi_init.cpp; then
    echo "✅ NAPI ResourceManager 集成"
else
    echo "❌ NAPI ResourceManager 未集成"
fi

# 检查文件复制逻辑是否已移除
if grep -q "copyAssetsToFiles" entry/src/main/ets/pages/Index.ets; then
    echo "❌ 文件复制逻辑仍然存在"
else
    echo "✅ 文件复制逻辑已移除"
fi

# 检查测试音是否已移除
if grep -q "sin(phase)" entry/src/main/cpp/ohos_audio_backend.cpp; then
    echo "❌ 测试音逻辑仍然存在"
else
    echo "✅ 测试音逻辑已移除"
fi

echo ""
echo "=== 测试完成 ==="
echo ""
echo "下一步："
echo "1. 如果编译成功，可以在设备上测试"
echo "2. 检查 HiLog 输出确认 rawfile 读取是否正常"
echo "3. 验证音频和资源文件是否正常加载"