#!/bin/bash
# 生成 OHOS assets 文件列表

echo "// 自动生成的 assets 文件列表"
echo "// 运行 ohos/generate_asset_list.sh 重新生成"
echo "const assetFiles: string[] = ["

find assets -type f \( \
    -name "*.zim" -o \
    -name "*.meta" -o \
    -name "*.ttf" -o \
    -name "*.ini" -o \
    -name "*.txt" -o \
    -name "*.png" -o \
    -name "*.svg" -o \
    -name "*.wav" \
\) | sed 's|^assets/||' | sort | while read file; do
    echo "  '$file',"
done

echo "];"
