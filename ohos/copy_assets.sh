#!/bin/bash
# 复制 PPSSPP assets 到 OHOS rawfile 目录

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PPSSPP_ROOT="$(dirname "$SCRIPT_DIR")"
ASSETS_SRC="$PPSSPP_ROOT/assets"
ASSETS_DST="$SCRIPT_DIR/entry/src/main/resources/rawfile/assets"

echo "Copying PPSSPP assets to OHOS rawfile directory..."
echo "Source: $ASSETS_SRC"
echo "Destination: $ASSETS_DST"

# 创建目标目录
mkdir -p "$ASSETS_DST"

# 复制所有资源文件
cp -r "$ASSETS_SRC"/* "$ASSETS_DST/"

echo "Done! Assets copied to $ASSETS_DST"
echo ""
echo "Files copied:"
ls -la "$ASSETS_DST"
