# 黑屏问题诊断指南

## 当前状况

从日志看，只有资源文件加载，没有其他关键日志：
```
12-13 13:29:05.777 D PPSSPP_Rawfile: Read file: assets/compat.ini (46699 bytes)
12-13 13:29:05.788 D PPSSPP_Rawfile: Read file: assets/compatvr.ini (24434 bytes)
12-13 13:29:05.819 D PPSSPP_Rawfile: Read file: assets/langregion.ini (1297 bytes)
12-13 13:29:05.824 D PPSSPP_Rawfile: Read file: assets/ppge_atlas.zim (666530 bytes)
12-13 13:29:05.874 D PPSSPP_Rawfile: Read file: assets/ppge_atlas.meta (50242 bytes)
12-13 13:29:06.073 I PPSSPP_NAPI_PPSSPP: SetKeepScreenOn called: 1
```

## 缺失的关键日志

应该看到但没有看到的日志：

### 1. 初始化日志
```
I PPSSPP_App: Initializing PPSSPP for HarmonyOS...
I PPSSPP_App: Calling NativeInit...
I PPSSPP_App: NativeInit completed
I PPSSPP_App: PPSSPP initialized successfully
```

### 2. 图形初始化日志
```
I PPSSPP_XComponent: OnSurfaceCreated called
I PPSSPP_XComponent: Creating graphics context...
I PPSSPP_System: NativeInitGraphics
I PPSSPP_G3D: Graphics context initialized
```

### 3. 渲染循环日志
```
I PPSSPP_G3D: Entering main render loop
I PPSSPP_XComponent: Frame 0
I PPSSPP_XComponent: Frame 100
```

### 4. UI 系统日志
```
I PPSSPP_System: ScreenManager created
I PPSSPP_System: Current screen: MainScreen
```

## 可能的原因

### 原因 1: 应用未初始化
**症状**: 没有看到 "Initializing PPSSPP" 日志

**检查**:
```bash
hdc shell hilog | grep "PPSSPP_App"
```

**解决**: 确保 ArkTS 调用了 `ppsspp.initEmulator()`

### 原因 2: Surface 未创建
**症状**: 没有看到 "OnSurfaceCreated" 日志

**检查**:
```bash
hdc shell hilog | grep "OnSurfaceCreated"
```

**解决**: 检查 XComponent 是否正确初始化

### 原因 3: 渲染循环未启动
**症状**: 没有看到 "Entering main render loop" 日志

**检查**:
```bash
hdc shell hilog | grep "render loop"
```

**解决**: 检查渲染线程是否启动

### 原因 4: UI 系统未初始化
**症状**: 没有看到 "ScreenManager" 相关日志

**检查**:
```bash
hdc shell hilog | grep "ScreenManager"
```

**解决**: 检查 NativeInit 是否完成

## 诊断步骤

### 步骤 1: 启用详细日志

修改 `ohos/entry/src/main/cpp/ohos_xcomponent.cpp`，取消注释日志：

```cpp
// 主渲染循环
int frameNum = 0;
while (!g_exitRenderLoop.load()) {
    // 启用帧日志
    if (frameNum % 100 == 0) {
        OHOS_LOGI(XCOMP_TAG, "Frame %{public}d", frameNum);
    }
    frameNum++;
    
    // 渲染一帧
    NativeFrame(ctx);
    // ...
}
```

### 步骤 2: 添加初始化日志

在 `UI/NativeApp.cpp` 的 `NativeInit` 中添加日志：

```cpp
void NativeInit(int argc, const char *argv[], const char *savegame_dir, const char *external_dir, const char *cache_dir) {
    INFO_LOG(Log::System, "=== NativeInit START ===");
    INFO_LOG(Log::System, "argc: %d", argc);
    for (int i = 0; i < argc; i++) {
        INFO_LOG(Log::System, "argv[%d]: %s", i, argv[i]);
    }
    // ... 原有代码
    INFO_LOG(Log::System, "=== NativeInit END ===");
}
```

### 步骤 3: 添加 ScreenManager 日志

在 `UI/NativeApp.cpp` 中：

```cpp
g_screenManager = new ScreenManager();
INFO_LOG(Log::System, "ScreenManager created: %p", g_screenManager);
```

### 步骤 4: 添加 NativeFrame 日志

在 `UI/NativeApp.cpp` 的 `NativeFrame` 开始处：

```cpp
void NativeFrame(GraphicsContext *graphicsContext) {
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {
        INFO_LOG(Log::G3D, "NativeFrame called, frame %d", frameCount);
    }
    // ... 原有代码
}
```

### 步骤 5: 重新编译并测试

```bash
cd ohos
rm -rf entry/.cxx
hvigorw assembleHap
hdc install entry/build/default/outputs/default/entry-default-signed.hap
```

### 步骤 6: 查看完整日志

```bash
# 清空日志
hdc shell hilog -c

# 启动应用

# 查看所有 PPSSPP 相关日志
hdc shell hilog | grep PPSSPP
```

## 预期的完整日志流程

正常情况下应该看到以下日志序列：

```
1. 应用启动
I PPSSPP_NAPI_PPSSPP: InitEmulator called
I PPSSPP_App: Initializing PPSSPP for HarmonyOS...

2. 系统初始化
I PPSSPP_System: OhosSystem initialized
I PPSSPP_Input: OhosInput initialized
I PPSSPP_Vibration: OhosVibration initialized

3. NativeInit
I PPSSPP_System: === NativeInit START ===
I PPSSPP_System: argc: 1
I PPSSPP_System: argv[0]: ppsspp
I PPSSPP_System: === NativeInit END ===

4. Surface 创建
I PPSSPP_XComponent: === OnSurfaceCreated START ===
I PPSSPP_XComponent: Creating graphics context...
I PPSSPP_XComponent: Surface size: 1080x1920

5. 图形初始化
I PPSSPP_System: NativeInitGraphics
I PPSSPP_G3D: Graphics context initialized

6. ScreenManager
I PPSSPP_System: ScreenManager created: 0x...
I PPSSPP_System: Current screen: MainScreen

7. 渲染循环
I PPSSPP_G3D: Entering main render loop
I PPSSPP_XComponent: Frame 0
I PPSSPP_G3D: NativeFrame called, frame 60
I PPSSPP_XComponent: Frame 100

8. UI 显示
I PPSSPP_System: Rendering MainScreen
I PPSSPP_UI: Drawing UI elements
```

## 快速检查清单

- [ ] 看到 "InitEmulator called" 日志
- [ ] 看到 "Initializing PPSSPP" 日志
- [ ] 看到 "NativeInit" 日志
- [ ] 看到 "OnSurfaceCreated" 日志
- [ ] 看到 "NativeInitGraphics" 日志
- [ ] 看到 "ScreenManager created" 日志
- [ ] 看到 "Entering main render loop" 日志
- [ ] 看到 "NativeFrame called" 日志
- [ ] 看到 "Frame" 计数日志

## 常见问题修复

### 问题: 没有任何 PPSSPP 日志

**原因**: HiLog 过滤器设置问题

**解决**:
```bash
# 设置日志级别
hdc shell hilog -b D

# 或者查看所有日志
hdc shell hilog
```

### 问题: 只有资源加载日志

**原因**: NativeInit 可能失败或未完成

**解决**: 检查 NativeInit 的返回值和错误日志

### 问题: 有初始化日志但无渲染日志

**原因**: 渲染循环未启动或 Surface 未创建

**解决**: 检查 OnSurfaceCreated 是否被调用

### 问题: 有渲染日志但黑屏

**原因**: UI 系统未正确渲染或 ScreenManager 问题

**解决**: 检查 ScreenManager 和当前屏幕状态

## 下一步

根据日志输出，确定问题所在：

1. **无初始化日志** → 检查 ArkTS 层的 initEmulator 调用
2. **无 Surface 日志** → 检查 XComponent 配置
3. **无渲染日志** → 检查渲染线程启动
4. **无 UI 日志** → 检查 ScreenManager 和屏幕栈

## 临时解决方案

如果问题复杂，可以先实现一个简单的测试：

```cpp
// 在 NativeFrame 中添加简单的渲染测试
void NativeFrame(GraphicsContext *graphicsContext) {
    static int testFrame = 0;
    if (testFrame++ < 10) {
        INFO_LOG(Log::G3D, "TEST: NativeFrame %d, context: %p", testFrame, graphicsContext);
    }
    
    // 测试：清屏为红色
    if (graphicsContext && graphicsContext->GetDrawContext()) {
        auto draw = graphicsContext->GetDrawContext();
        draw->Clear(0, 0, 0, 1.0f, 1.0f, 0.0f, 0);  // 红色
    }
    
    // 原有代码...
}
```

如果看到红色屏幕，说明渲染管线工作，问题在 UI 系统。
如果还是黑屏，说明渲染管线有问题。
