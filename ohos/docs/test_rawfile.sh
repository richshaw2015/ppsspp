#!/bin/bash
# 测试 Rawfile 直接读取方案

echo "=== PPSSPP OHOS Rawfile 直接读取测试 ==="
echo ""

# 1. 卸载旧版本
echo "=== 1. 卸载旧版本 ==="
hdc shell bm uninstall -n com.ppsspp.ppsspp
echo "✅ 已卸载"
echo ""

# 2. 重新编译
echo "=== 2. 重新编译 ==="
./rebuild.sh
if [ $? -ne 0 ]; then
    echo "❌ 编译失败！"
    exit 1
fi
echo "✅ 编译成功"
echo ""

# 3. 安装应用
echo "=== 3. 安装应用 ==="
hdc install entry/build/default/outputs/default/entry-default-signed.hap
if [ $? -ne 0 ]; then
    echo "❌ 安装失败！"
    exit 1
fi
echo "✅ 安装成功"
echo ""

# 4. 清空日志
echo "=== 4. 清空日志缓冲区 ==="
hdc shell hilog -r
echo "✅ 日志已清空"
echo ""

# 5. 等待用户启动应用
echo "=== 5. 请在设备上启动 PPSSPP ==="
echo ""
echo "启动后，应用会直接从 rawfile 读取资源（无需复制）"
echo "按回车键开始监控日志..."
read

# 6. 监控日志
echo "=== 6. 监控 Rawfile 和 VFS 日志 ==="
echo ""

# 等待 3 秒让应用启动
sleep 3

# 检查关键日志
echo "检查 ResourceManager 初始化："
hdc shell hilog -x | grep "ResourceManager" | head -5

echo ""
echo "检查 Rawfile Reader 注册："
hdc shell hilog -x | grep "Rawfile" | head -5

echo ""
echo "检查 VFS 注册："
hdc shell hilog -x | grep "VFS.*Register\|Assets registered" | head -5

echo ""
echo "=== 7. 测试文件读取 ==="
echo ""
echo "查看 Rawfile 读取日志："
hdc shell hilog -x | grep "Read file:" | head -10

echo ""
echo "=== 8. 现在请执行以下操作 ==="
echo ""
echo "1. 在 PPSSPP 中进入 设置 → 音频"
echo "2. 勾选 'UI sound'"
echo "3. 返回主菜单"
echo "4. 点击任意按钮"
echo ""
echo "按回车键开始监控音频..."
read

# 7. 监控音频
echo "=== 9. 监控音频数据 ==="
echo "按 Ctrl+C 停止监控"
echo "----------------------------------------"

hdc shell hilog -x | grep -E "Max sample|AudioCallback|sfx" --color=always

