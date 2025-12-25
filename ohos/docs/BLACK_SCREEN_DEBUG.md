# 黑屏问题排查指南

## 问题描述
游戏加载后出现黑屏，没有画面显示。

## 已修复的问题

### 1. 删除了 stub 实现
**问题**：`ohos_native_app_stubs.cpp` 中的空实现覆盖了 `UI/NativeApp.cpp` 中的真正实现。

**修复**：
- 删除了 `ohos/entry/src/main/cpp/ohos_native_app_stubs.cpp`
- 确保使用 `UI/NativeApp.cpp` 中的完整实现

### 2. 实现了游戏加载
**问题**：`LoadGame()` 函数是空实现，游戏没有真正加载。

**修复**：
```cpp
bool LoadGame(const std::string& gamePath) {
    // 使用 PPSSPP 的 NativeMessageReceived 来加载游戏
    std::string bootCommand = "boot:" + gamePath;
    NativeMessageReceived(bootCommand.c_str(), "");
    
    g_running.store(true);
    g_paused.store(false);
    
    return true;
}
```

## 排查步骤

### 1. 检查编译
重新编译项目，确保使用正确的实现：
```bash
cd ohos
rm -rf entry/.cxx
hvigorw assembleHap
```

### 2. 检查日志 - 图形初始化

查看图形系统是否正确初始化：
```bash
hdc shell hilog | grep -E "NativeInitGraphics|GraphicsContext"
```

**预期日志**：
```
I/PPSSPP_System: NativeInitGraphics
I/PPSSPP_G3D: Graphics context initialized
```

**如果看到**：
```
I/PPSSPP_System: NativeInitGraphics (stub)
```
说明还在使用 stub 实现，需要清理构建并重新编译。

### 3. 检查日志 - 渲染循环

查看渲染循环是否运行：
```bash
hdc shell hilog | grep -E "NativeFrame|render"
```

**预期**：应该看到持续的渲染日志（如果启用了调试日志）。

### 4. 检查日志 - 游戏加载

查看游戏是否成功加载：
```bash
hdc shell hilog | grep -E "boot:|Loading game|PSP_CoreParameter"
```

**预期日志**：
```
I/PPSSPP_App: Loading game: /path/to/game.iso
I/PPSSPP_App: Game load command sent
I/PPSSPP_System: Received message: boot:/path/to/game.iso
```

### 5. 检查日志 - 显示参数

查看显示参数是否正确设置：
```bash
hdc shell hilog | grep -E "Display:|Recalculate|NativeResized"
```

**预期日志**：
```
I/PPSSPP_XComponent: Display: pixel=1080x1920, dp=360x640
I/PPSSPP_XComponent: Calling NativeResized...
I/PPSSPP_XComponent: NativeResized completed
```

### 6. 检查日志 - 错误信息

查看是否有错误：
```bash
hdc shell hilog | grep -E "ERROR|FATAL|failed"
```

## 常见问题

### 问题 1: 还在使用 stub 实现

**症状**：
```
I/PPSSPP_System: NativeInitGraphics (stub)
```

**原因**：
- 构建缓存没有清理
- stub 文件还在被编译

**解决**：
```bash
# 完全清理构建
cd ohos
rm -rf entry/.cxx
rm -rf entry/build

# 确认 stub 文件已删除
ls ohos/entry/src/main/cpp/ohos_native_app_stubs.cpp
# 应该显示 "No such file or directory"

# 重新构建
hvigorw clean
hvigorw assembleHap
```

### 问题 2: 游戏没有加载

**症状**：
- 黑屏
- 没有 "boot:" 相关日志

**原因**：
- `LoadGame()` 没有被调用
- 游戏路径错误
- 文件不存在

**解决**：
1. 确认游戏文件存在且可访问
2. 检查文件路径是否正确
3. 检查文件权限

### 问题 3: 图形上下文未初始化

**症状**：
```
E/PPSSPP_G3D: Graphics context is null
```

**原因**：
- OpenGL/Vulkan 初始化失败
- Surface 未创建

**解决**：
1. 检查 `OnSurfaceCreated` 是否被调用
2. 检查 OpenGL/Vulkan 初始化日志
3. 尝试切换渲染后端（OpenGL ↔ Vulkan）

### 问题 4: 显示参数错误

**症状**：
- 黑屏
- 或者 UI 显示异常

**原因**：
- `g_display.Recalculate()` 未调用
- DPI 参数错误
- 分辨率为 0

**解决**：
确保 `OnSurfaceCreated` 中正确设置了显示参数：
```cpp
g_display.Recalculate(width, height, dpi_scale_x, dpi_scale_y, uiScaleMultiplier);
NativeResized();
```

### 问题 5: UI 系统未初始化

**症状**：
- 黑屏
- 没有 UI 元素

**原因**：
- `NativeInit()` 未完成
- UI 资源加载失败
- 字体加载失败

**解决**：
1. 检查 `NativeInit()` 的日志
2. 确认资源文件存在（assets/）
3. 检查字体文件是否正确加载

## 调试技巧

### 1. 启用详细日志

在 `ohos_app.cpp` 的 `Initialize()` 中：
```cpp
// 启用所有模块的详细日志
g_logManager.SetAllLogLevels(LogLevel::LDEBUG);
```

### 2. 添加调试日志

在关键位置添加日志：

**NativeFrame 开始**：
```cpp
void NativeFrame(GraphicsContext *graphicsContext) {
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {
        INFO_LOG(Log::G3D, "NativeFrame called, frame %d", frameCount);
    }
    // ...
}
```

**游戏加载**：
```cpp
bool LoadGame(const std::string& gamePath) {
    INFO_LOG(Log::System, "LoadGame called: %s", gamePath.c_str());
    // ...
}
```

### 3. 检查 ScreenManager

添加日志检查屏幕管理器状态：
```cpp
if (g_screenManager) {
    INFO_LOG(Log::System, "Current screen: %s", 
             g_screenManager->topScreen()->tag());
}
```

### 4. 检查渲染标志

在 `NativeFrame` 中检查渲染标志：
```cpp
ScreenRenderFlags renderFlags = g_screenManager->render();
INFO_LOG(Log::G3D, "Render flags: %d", (int)renderFlags);
```

## 验证修复

### 成功标准

1. ✅ 看到 `NativeInitGraphics` 日志（不是 stub）
2. ✅ 看到 `NativeFrame` 被持续调用
3. ✅ 看到游戏加载日志 `boot:...`
4. ✅ 看到显示参数设置日志
5. ✅ 看到 UI 渲染日志
6. ✅ 屏幕显示内容（即使是 UI 菜单也算成功）

### 测试步骤

1. **测试 UI 显示**：
   - 启动应用
   - 应该看到 PPSSPP 主菜单
   - 如果看到主菜单 = UI 系统工作正常

2. **测试游戏加载**：
   - 选择一个游戏
   - 点击启动
   - 应该看到游戏画面或加载画面

3. **测试渲染**：
   - 游戏运行时应该有画面更新
   - FPS 应该 > 0
   - 画面应该响应输入

## 下一步

如果黑屏问题解决但游戏还有其他问题：

1. **性能问题** → 检查 FPS、优化渲染
2. **输入问题** → 检查 `ohos_input.cpp`
3. **音频问题** → 检查 `ohos_audio.cpp`
4. **崩溃问题** → 检查崩溃日志和堆栈

## 相关文档

- `MEMORY_INIT_FIX.md` - 内存初始化问题
- `OPENGL_IMPLEMENTATION.md` - OpenGL 渲染
- `VULKAN_IMPLEMENTATION.md` - Vulkan 渲染
- `RENDER_LOOP.md` - 渲染循环详解
- `UI_INTEGRATION.md` - UI 系统集成

## 总结

黑屏问题通常是由以下原因造成的：

1. ❌ 使用了 stub 实现而不是真正的 UI 代码
2. ❌ 游戏没有真正加载
3. ❌ 图形上下文未初始化
4. ❌ 显示参数未设置
5. ❌ 渲染循环未运行

通过删除 stub 文件、实现游戏加载、确保正确的编译链接，这些问题应该都能解决。
