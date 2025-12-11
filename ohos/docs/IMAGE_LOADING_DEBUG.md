# OHOS 图片资源加载诊断

## 问题现象
- 字体能正常显示（说明字体图集加载成功）
- UI 图片无法显示（PNG 图片加载失败）

## 资源加载流程

### 1. 资源复制流程（ArkTS 层）
`Index.ets` 中的 `copyAssetsToFiles()` 函数：
```
rawfile/assets/ui_images/*.png 
  → context.filesDir/assets/ui_images/*.png
```

### 2. VFS 注册（Native 层）
`UI/NativeApp.cpp` 中的 `NativeInit()` 函数：
```cpp
Path ohosAssetsPath = Path(savegame_dir) / "assets";
g_VFS.Register("", new DirectoryReader(ohosAssetsPath));
```

### 3. 图片加载（Native 层）
`Common/Render/AtlasGen.cpp` 中的 `Image::LoadPNG()` 函数：
```cpp
const uint8_t *file_data = g_VFS.ReadFile(png_name, &sz);
// png_name 例如: "ui_images/icon.png"
```

## 可能的问题点

### ✓ 已确认正常的部分
1. 资源文件已复制到 rawfile 目录
2. VFS 注册代码已添加
3. 路径参数从 ArkTS 正确传递到 Native

### ❓ 需要检查的部分

#### 1. 资源文件是否被复制到设备
- 检查 ArkTS 日志：`[PPSSPP] Copied: ui_images/icon.png`
- 检查标记文件：`context.filesDir/assets/ui_images/icon.png`

#### 2. VFS 是否正确注册
- 检查 Native 日志：`OHOS: ✓ Registered assets from ...`
- 检查测试文件：`OHOS: ✓ Found test file: ui_images/icon.png`

#### 3. VFS ReadFile 是否成功
- 在 `Image::LoadPNG()` 中添加日志
- 检查 `g_VFS.ReadFile()` 返回值

#### 4. PNG 解码是否成功
- 检查 `pngLoadPtr()` 返回值
- 验证 PNG 文件格式是否正确

## 调试步骤

### 步骤 1: 验证资源复制
在设备上运行应用，查看日志：
```
[PPSSPP] Copying assets...
[PPSSPP] Copied: ui_images/icon.png
[PPSSPP] Copied: ui_images/images.svg
...
[PPSSPP] Asset copy completed
```

### 步骤 2: 验证 VFS 注册
查看 Native 日志：
```
OHOS: Checking assets path: /data/storage/.../files/assets
OHOS: ✓ Registered assets from /data/storage/.../files/assets
OHOS: ✓ Found test file: ui_images/icon.png
```

### 步骤 3: 验证图片加载
查看图片加载日志：
```
Failed to load png from VFS: ui_images/xxx.png  // 如果 VFS 读取失败
Failed to load ui_images/xxx.png                // 如果 PNG 解码失败
```

## 已添加的调试日志

### UI/NativeApp.cpp
```cpp
INFO_LOG(Log::System, "OHOS: Checking assets path: %s", ohosAssetsPath.c_str());
INFO_LOG(Log::System, "OHOS: ✓ Registered assets from %s", ohosAssetsPath.c_str());
INFO_LOG(Log::System, "OHOS: ✓ Found test file: ui_images/icon.png");
WARN_LOG(Log::System, "OHOS: ✗ Missing test file: ui_images/icon.png");
ERROR_LOG(Log::System, "OHOS: ✗ Assets directory not found: %s", ohosAssetsPath.c_str());
```

### Common/Render/AtlasGen.cpp
```cpp
printf("Failed to load png from VFS: %s\n", png_name);
printf("Failed to load %s\n", png_name);
```

## 下一步行动

1. **重新构建并运行应用**
2. **收集完整日志**，特别关注：
   - `[PPSSPP]` 开头的 ArkTS 日志
   - `OHOS:` 开头的 Native 日志
   - `Failed to load` 相关的错误日志
3. **根据日志定位问题**：
   - 如果资源复制失败 → 检查 `Index.ets` 的 `copyAssetsToFiles()`
   - 如果 VFS 注册失败 → 检查路径是否正确
   - 如果 VFS 读取失败 → 检查 VFS 注册顺序和路径
   - 如果 PNG 解码失败 → 检查 PNG 文件格式

## 参考其他平台

### Android 平台
Android 不需要手动复制资源，因为它使用 APK 的 assets 目录：
```cpp
// Android 直接从 APK 读取
g_VFS.Register("", new ZipAssetReader(apkPath, "assets/"));
```

### iOS 平台
iOS 使用 app bundle：
```cpp
g_VFS.Register("", new DirectoryReader(Path(external_dir)));
```

### OHOS 平台（当前实现）
类似 Android，但需要手动复制 rawfile 到 filesDir：
```cpp
// 1. ArkTS: rawfile → filesDir/assets
// 2. Native: VFS 注册 filesDir/assets
g_VFS.Register("", new DirectoryReader(ohosAssetsPath));
```
