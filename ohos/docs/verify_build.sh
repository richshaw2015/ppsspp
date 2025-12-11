#!/bin/bash

# PPSSPP 鸿蒙版本构建验证脚本

set -e

echo "========================================="
echo "PPSSPP 鸿蒙版本构建验证"
echo "========================================="
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查函数
check_command() {
    if command -v $1 &> /dev/null; then
        echo -e "${GREEN}✓${NC} $1 已安装"
        return 0
    else
        echo -e "${RED}✗${NC} $1 未安装"
        return 1
    fi
}

check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} 文件存在: $1"
        return 0
    else
        echo -e "${RED}✗${NC} 文件缺失: $1"
        return 1
    fi
}

check_dir() {
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} 目录存在: $1"
        return 0
    else
        echo -e "${RED}✗${NC} 目录缺失: $1"
        return 1
    fi
}

# 1. 检查开发环境
echo "1. 检查开发环境..."
echo "-------------------"

check_command "hdc" || echo -e "${YELLOW}  提示: 请安装 HarmonyOS SDK${NC}"
check_command "node" || echo -e "${YELLOW}  提示: 请安装 Node.js${NC}"

echo ""

# 2. 检查项目结构
echo "2. 检查项目结构..."
echo "-------------------"

check_dir "entry"
check_dir "entry/src/main/cpp"
check_dir "entry/src/main/ets"
check_file "entry/src/main/cpp/CMakeLists.txt"
check_file "entry/build-profile.json5"
check_file "build-profile.json5"

echo ""

# 3. 检查 Native 源文件
echo "3. 检查 Native 源文件..."
echo "-------------------"

check_file "entry/src/main/cpp/napi/napi_init.cpp"
check_file "entry/src/main/cpp/napi/napi_ppsspp.cpp"
check_file "entry/src/main/cpp/napi/napi_ppsspp.h"
check_file "entry/src/main/cpp/ohos_app.cpp"
check_file "entry/src/main/cpp/ohos_app.h"
check_file "entry/src/main/cpp/ohos_system.cpp"
check_file "entry/src/main/cpp/ohos_system.h"
check_file "entry/src/main/cpp/ohos_audio.cpp"
check_file "entry/src/main/cpp/ohos_audio.h"
check_file "entry/src/main/cpp/ohos_gl_context.cpp"
check_file "entry/src/main/cpp/ohos_gl_context.h"
check_file "entry/src/main/cpp/ohos_input.cpp"
check_file "entry/src/main/cpp/ohos_input.h"

echo ""

# 4. 检查 ArkTS 源文件
echo "4. 检查 ArkTS 源文件..."
echo "-------------------"

check_file "entry/src/main/ets/napi/PPSSPPNative.ets"
check_file "entry/src/main/ets/pages/GameView.ets"

echo ""

# 5. 检查 PPSSPP 核心文件
echo "5. 检查 PPSSPP 核心文件..."
echo "-------------------"

cd ..
check_dir "Common"
check_dir "Core"
check_dir "ext"
check_file "CMakeLists.txt"
cd ohos

echo ""

# 6. 尝试编译（如果有 hvigorw）
echo "6. 尝试编译..."
echo "-------------------"

if [ -f "hvigorw" ]; then
    echo "开始编译..."
    if ./hvigorw assembleHap --no-daemon 2>&1 | tee build.log; then
        echo -e "${GREEN}✓${NC} 编译成功"
        
        # 检查输出文件
        if [ -f "entry/build/default/outputs/default/entry-default-signed.hap" ]; then
            HAP_SIZE=$(du -h entry/build/default/outputs/default/entry-default-signed.hap | cut -f1)
            echo -e "${GREEN}✓${NC} HAP 文件已生成 (大小: $HAP_SIZE)"
        fi
    else
        echo -e "${RED}✗${NC} 编译失败，请查看 build.log"
        exit 1
    fi
else
    echo -e "${YELLOW}⚠${NC} hvigorw 不存在，跳过编译测试"
fi

echo ""

# 7. 检查设备连接
echo "7. 检查设备连接..."
echo "-------------------"

if command -v hdc &> /dev/null; then
    DEVICES=$(hdc list targets 2>/dev/null)
    if [ -n "$DEVICES" ]; then
        echo -e "${GREEN}✓${NC} 已连接设备:"
        echo "$DEVICES"
    else
        echo -e "${YELLOW}⚠${NC} 未检测到设备"
    fi
else
    echo -e "${YELLOW}⚠${NC} hdc 命令不可用"
fi

echo ""

# 8. 总结
echo "========================================="
echo "验证完成"
echo "========================================="
echo ""
echo "下一步:"
echo "1. 如果编译成功，运行: ./hvigorw installHapDebug"
echo "2. 查看日志: hdc shell hilog | grep PPSSPP"
echo "3. 参考 QUICK_START.md 继续开发"
echo ""
