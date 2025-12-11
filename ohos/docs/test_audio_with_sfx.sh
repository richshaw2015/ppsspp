#!/bin/bash
# 测试音频（包含音效文件）

echo "=== PPSSPP OHOS 音频测试（含音效文件）==="
echo ""

# 1. 卸载旧版本
echo "=== 1. 卸载旧版本（清除数据）==="
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
echo "启动后，应用会自动复制资源文件（包括音效文件）"
echo "请等待几秒钟..."
echo ""
echo "按回车键开始监控日志..."
read

# 6. 监控日志
echo "=== 6. 监控音效文件复制 ==="
echo ""

# 等待 3 秒让应用启动
sleep 3

# 检查音效文件是否复制
echo "检查音效文件复制情况："
hdc shell hilog -x | grep "Copied.*sfx" | head -10

echo ""
echo "=== 7. 现在请执行以下操作 ==="
echo ""
echo "1. 在 PPSSPP 中进入 设置 → 音频"
echo "2. 勾选 'UI sound'"
echo "3. 确认 'UI volume' > 0"
echo "4. 返回主菜单"
echo "5. 点击任意按钮"
echo ""
echo "按回车键开始监控音频数据..."
read

# 7. 监控音频数据
echo "=== 8. 监控音频数据 ==="
echo "查找非零的 Max sample 值..."
echo "按 Ctrl+C 停止监控"
echo "----------------------------------------"

hdc shell hilog -x | grep -E "Max sample|AudioCallback" --color=always

