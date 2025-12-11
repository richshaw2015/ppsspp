#!/bin/bash
# PPSSPP OHOS 重新编译脚本

echo "=== 清理构建缓存 ==="
rm -rf entry/.cxx
rm -rf .hvigor
rm -rf entry/build

echo "=== 清理完成 ==="

echo "=== 开始编译 ==="
hvigorw clean
hvigorw assembleHap

echo "=== 编译完成 ==="
echo ""
echo "安装命令："
echo "  hdc install entry/build/default/outputs/default/entry-default-signed.hap"
echo ""
echo "查看日志："
echo "  hdc shell hilog -x | grep -E 'PPSSPP|NativeInit|XComponent'"
