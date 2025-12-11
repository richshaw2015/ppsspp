# OHOS 资源加载问题修复总结

## 问题描述

PPSSPP 在 OHOS 平台启动后：
- ✅ 字体能正常显示
- ❌ UI 图片无法显示

## 根本原因

资源文件复制不完整。标记文件检查逻辑存在问题，导致：
1. 首次复制可能不完整
2. 后续启动时错误地认为资源已复制，跳过复制
3. Native 层无法找到图片文件

## 解决方案

### 1. 改进标记文件检查逻辑

**之前的问题：**
```typescript
// 只检查文件是否存在，不验证有效性
fs.accessSync(markerFile);
```

**修复后：**
```typescript
// 检查文件大小，确保不是空文件
const stat = fs.statSync(markerFile);
if (stat.size > 0) {
    // 文件有效，跳过复制
    return;
}
```

### 2. 完善资源文件列表

添加了缺失的 `images.svg` 文件：
```typescript
'ui_images/images.svg',  // SVG 图集源文件
```

### 3. 规范化路径处理

**移除硬编码路径：**
```cpp
// ❌ 错误
g_dataDir = "/data/storage/el2/base/haps/entry/files";

// ✅ 正确
bool Initialize(const std::string& dataDir, const std::string& cacheDir) {
    g_dataDir = dataDir;  // 来自 context.filesDir
}
```

### 4. 统一日志系统

使用 `ohos_hilog.h` 包装器避免 `LogLevel` 命名冲突：
```cpp
#if defined(OHOS) || defined(__OHOS__)
#include "../ohos/entry/src/main/cpp/ohos_hilog.h"
#endif
```

## 资源加载流程

```
┌─────────────────────────────────────────────────────────────┐
│ 1. 构建时：copy_assets.sh                                   │
│    assets/* → ohos/entry/src/main/resources/rawfile/assets/ │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 2. 运行时（ArkTS）：Index.ets                                │
│    rawfile/assets/* → context.filesDir/assets/*             │
│    - 检查标记文件 ui_images/icon.png                         │
│    - 验证文件大小 > 0                                        │
│    - 如果不存在或无效，复制所有资源                           │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 3. Native 层：UI/NativeApp.cpp                              │
│    g_VFS.Register("", new DirectoryReader(                  │
│        Path(savegame_dir) / "assets"));                     │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 4. 图片加载：Common/Render/AtlasGen.cpp                     │
│    g_VFS.ReadFile("ui_images/icon.png", &sz);              │
└─────────────────────────────────────────────────────────────┘
```

## 验证结果

修复后的日志输出：
```
OHOS: ✓ Registered assets from <filesDir>/assets
OHOS: ✓ Found test file: ui_images/icon.png
```

## 关键文件修改

### ArkTS 层
- `ohos/entry/src/main/ets/pages/Index.ets`
  - 改进标记文件检查逻辑
  - 添加文件大小验证
  - 添加 `images.svg` 到资源列表

### Native 层
- `UI/NativeApp.cpp`
  - 添加 OHOS 平台的 VFS 注册
  - 使用 `ohos_hilog.h` 进行日志输出
  - 移除硬编码路径

- `ohos/entry/src/main/cpp/napi/napi_init.cpp`
  - 接收 `filesDir` 和 `cacheDir` 参数
  - 验证参数有效性
  - 初始化 `OhosSystem`

- `ohos/entry/src/main/cpp/ohos_system.cpp`
  - 移除硬编码路径
  - 从参数接收路径

### 构建配置
- `ohos/entry/src/main/cpp/CMakeLists.txt`
  - UI 库添加 `OHOS=1` 宏定义
  - 确保平台检测正确

## 最佳实践

### 1. 路径处理
- ✅ 使用 `context.filesDir` 和 `context.cacheDir`
- ❌ 不要硬编码 `/data/storage/...` 路径

### 2. 资源复制
- ✅ 验证标记文件的大小和有效性
- ❌ 不要只检查文件是否存在

### 3. 日志输出
- ✅ 使用 `ohos_hilog.h` 包装器
- ❌ 不要直接包含 `hilog/log.h`

### 4. 平台检测
- ✅ 使用 `#if defined(OHOS)` 或 `#if PPSSPP_PLATFORM(OHOS)`
- ✅ 确保目标库定义了 `OHOS=1` 宏

## 相关文档

- `ohos/HILOG_USAGE.md` - HiLog 使用指南
- `ohos/IMAGE_LOADING_DEBUG.md` - 图片加载诊断
- `ohos/CURRENT_STATUS.md` - 当前状态总结

## 后续优化建议

1. **增量复制**：只复制缺失或更新的文件
2. **校验和验证**：使用 MD5/SHA 验证文件完整性
3. **压缩传输**：考虑使用 ZIP 打包资源
4. **懒加载**：按需加载资源而非一次性全部复制
