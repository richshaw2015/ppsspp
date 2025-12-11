# PPSSPP OHOS 平台当前状态

## 最新进展（2024-12-09）

### ✅ 已完成的工作

1. **路径处理规范化**
   - 移除所有硬编码路径（`/data/storage/...`）
   - 路径从 ArkTS 的 `context.filesDir` 和 `context.cacheDir` 传入
   - `OhosSystem::Initialize()` 接收并保存路径参数
   - `napi_init.cpp` 验证路径参数并初始化系统

2. **HiLog 日志系统**
   - 解决了 `LogLevel` 命名冲突问题
   - 创建了 `ohos_hilog.h` 包装器
   - 在 `UI/NativeApp.cpp` 中成功使用 HiLog
   - 创建了 `HILOG_USAGE.md` 使用指南

3. **资源加载准备**
   - 添加了 `images.svg` 到资源复制列表
   - 在 `UI/NativeApp.cpp` 中添加了 OHOS 平台的 VFS 注册逻辑
   - 添加了资源路径验证和调试日志

4. **CMake 配置**
   - UI 库添加了 `OHOS=1` 宏定义
   - 确保跨平台代码能正确识别 OHOS 平台

### ✅ 最新完成（2024-12-09 下午）

5. **触摸输入实现**
   - 实现 `OnTouchEvent` 函数处理触摸事件
   - 支持多点触控（最多 10 个触摸点）
   - 坐标转换和 DPI 缩放
   - 触摸动作映射（DOWN/MOVE/UP/CANCEL）
   - 集成到 PPSSPP 输入系统

6. **UI 优化**
   - 移除顶部状态栏（"PPSSPP Running"）
   - 全屏显示 PPSSPP 界面

### 📋 下一步行动

#### 1. 分析日志输出

查看最新的日志，确认：
```
- NativeInit CALLED
- Platform Detection Debug
- OHOS PLATFORM DETECTED
- Checking assets path
- Registered assets from ...
- Found test file: ui_images/icon.png
```

#### 2. 根据日志结果采取行动

**如果看到 "OHOS PLATFORM DETECTED"：**
- ✅ 平台检测成功
- 继续检查资源加载日志

**如果看到 "Assets directory not found"：**
- 检查 ArkTS 层的资源复制逻辑
- 验证 `context.filesDir` 路径是否正确

**如果看到 "Missing test file: ui_images/icon.png"：**
- 资源目录存在但文件缺失
- 检查 `Index.ets` 中的资源文件列表
- 验证 `copy_assets.sh` 脚本

**如果图片仍然无法显示：**
- 检查 `Common/Render/AtlasGen.cpp` 中的 PNG 加载逻辑
- 添加更多调试日志到 `Image::LoadPNG()`
- 验证 VFS 的 `ReadFile()` 是否成功

#### 3. 完善日志系统

将其他 OHOS 平台文件中的日志改为使用 `ohos_hilog.h`：
- `ohos_system.cpp`
- `ohos_audio.cpp`
- `ohos_input.cpp`
- `ohos_app.cpp`

### 🔧 技术要点

#### 路径处理规范
```cpp
// ❌ 错误：硬编码路径
g_dataDir = "/data/storage/el2/base/haps/entry/files";

// ✅ 正确：从 ArkTS 传入
bool Initialize(const std::string& dataDir, const std::string& cacheDir) {
    g_dataDir = dataDir;  // 来自 context.filesDir
    g_cacheDir = cacheDir; // 来自 context.cacheDir
}
```

#### HiLog 使用规范
```cpp
// 在文件开头
#if defined(OHOS) || defined(__OHOS__)
#include "../ohos/entry/src/main/cpp/ohos_hilog.h"
#define OHOS_LOG_TAG "ModuleName"
#endif

// 在代码中
#if defined(OHOS) || defined(__OHOS__)
    OHOS_LOGI(OHOS_LOG_TAG, "Message: %s", str);
#endif
```

#### 资源加载流程
```
1. ArkTS: rawfile → context.filesDir/assets/
2. Native: VFS 注册 filesDir/assets/
3. PPSSPP: g_VFS.ReadFile("ui_images/icon.png")
```

### 📝 相关文档

- `ohos/HILOG_USAGE.md` - HiLog 使用指南
- `ohos/IMAGE_LOADING_DEBUG.md` - 图片加载诊断
- `ohos/ARKTS_INTEGRATION.md` - ArkTS 集成文档

### 🐛 已知问题

1. **图片无法显示** - 正在诊断中
   - 字体能显示（字体图集加载成功）
   - UI 图片无法显示（PNG 加载失败）
   - 可能原因：VFS 路径、文件复制、PNG 解码

2. **日志级别** - 部分解决
   - PPSSPP 的 `INFO_LOG` 可能不输出
   - 使用 `OHOS_LOGI` 作为补充

### 💡 调试技巧

1. **查看完整日志**
   ```bash
   hdc shell hilog | grep "PPSSPP"
   ```

2. **检查文件是否存在**
   ```bash
   hdc shell ls -la /data/storage/el2/base/haps/entry/files/assets/ui_images/
   ```

3. **清理并重新安装**
   ```bash
   hdc uninstall app.superedu.psp
   hdc install entry-default-signed.hap
   ```

4. **查看特定模块日志**
   ```bash
   hdc shell hilog | grep "NativeApp"
   hdc shell hilog | grep "XComponent"
   ```
