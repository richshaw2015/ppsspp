# 渲染循环实现

**更新时间**: 2024-12-08  
**状态**: ✅ 已完成

## 概述

本文档描述 PPSSPP 鸿蒙版的渲染循环实现，参考 Android 的 `app-android.cpp` 实现。

## 架构

### 线程模型

```
主线程 (ArkTS)
    ↓
NAPI 线程
    ↓
渲染线程 (C++)
    ↓
OpenGL ES 渲染
```

### 渲染循环流程

```
1. XComponent 表面创建
   ↓
2. 启动渲染线程
   ↓
3. 初始化 GL 上下文
   ↓
4. 初始化 PPSSPP 图形系统
   ↓
5. 进入主循环
   ├─ 渲染一帧 (NativeFrame)
   ├─ 处理渲染任务 (ThreadFrame)
   ├─ 更新帧统计
   └─ 帧率控制
   ↓
6. 退出循环
   ↓
7. 清理资源
```

## 核心组件

### 1. 渲染线程

**文件**: `ohos/entry/src/main/cpp/ohos_xcomponent.cpp`

**关键变量**:
```cpp
static std::thread g_renderThread;              // 渲染线程
static std::atomic<bool> g_renderLoopRunning;   // 循环运行标志
static std::atomic<bool> g_exitRenderLoop;      // 退出标志
static bool g_rendererInited;                   // 渲染器初始化标志
```

**生命周期**:
1. `OnSurfaceCreated` → 启动渲染线程
2. 渲染线程运行主循环
3. `OnSurfaceDestroyed` → 停止渲染线程

### 2. 主渲染循环

```cpp
void RenderLoop() {
    g_renderThread = std::thread([]() {
        // 初始化
        g_glContext->InitFromRenderThread(...);
        NativeInitGraphics(g_glContext);
        g_glContext->ThreadStart();
        
        // 主循环
        while (!g_exitRenderLoop.load()) {
            // 渲染一帧
            NativeFrame(g_glContext);
            
            // 处理任务
            g_glContext->ThreadFrame(true);
            
            // 帧统计
            UpdateFrameStats();
            
            // 帧率控制
            FrameRateControl();
        }
        
        // 清理
        NativeShutdownGraphics();
        g_glContext->ThreadEnd();
        g_glContext->ShutdownFromRenderThread();
    });
}
```

### 3. 帧率控制

**目标**: 60 FPS (16.67ms per frame)

**实现**:
```cpp
double frameStartTime = time_now_d();

// 渲染...

double frameTime = time_now_d() - frameStartTime;
const double targetFrameTime = 1.0 / 60.0;  // 60 FPS

if (frameTime < targetFrameTime) {
    double sleepTime = targetFrameTime - frameTime;
    if (sleepTime > 0.001) {  // 1ms
        std::this_thread::sleep_for(
            std::chrono::microseconds(static_cast<int>(sleepTime * 1000000.0))
        );
    }
}
```

**特点**:
- 只在帧时间明显小于目标时才休眠
- 避免过度休眠导致的延迟
- 允许超过 60 FPS（如果硬件支持）

### 4. 帧统计

**统计指标**:
- 当前 FPS
- 上一帧渲染时间
- 帧计数

**实现**:
```cpp
// 帧率统计变量
static std::atomic<int> g_frameCount(0);
static std::atomic<double> g_lastFpsTime(0.0);
static std::atomic<double> g_currentFps(0.0);
static std::atomic<double> g_lastFrameTime(0.0);

// 更新统计
g_frameCount.fetch_add(1);
double currentTime = time_now_d();
double timeSinceLastFps = currentTime - g_lastFpsTime.load();

// 每秒更新一次 FPS
if (timeSinceLastFps >= 1.0) {
    int frameCount = g_frameCount.exchange(0);
    g_currentFps.store(frameCount / timeSinceLastFps);
    g_lastFpsTime.store(currentTime);
}

// 记录帧时间
g_lastFrameTime.store(currentTime - frameStartTime);
```

**日志输出**:
- 每 5 秒输出一次 FPS 到日志
- 格式: `FPS: 60.0`

## API 接口

### 渲染循环控制

#### RenderLoop()
启动渲染循环（在独立线程中）

**调用时机**: XComponent 表面创建时

**注意**: 
- 自动检查是否已运行
- 线程安全

#### StopRenderLoop()
停止渲染循环并等待线程结束

**调用时机**: XComponent 表面销毁时

**注意**:
- 会阻塞直到渲染线程完全退出
- 线程安全

### 帧统计查询

#### GetCurrentFPS()
```cpp
double GetCurrentFPS();
```
返回当前 FPS（每秒更新一次）

#### GetLastFrameTime()
```cpp
double GetLastFrameTime();
```
返回上一帧的渲染时间（秒）

#### GetSurfaceWidth() / GetSurfaceHeight()
```cpp
int GetSurfaceWidth();
int GetSurfaceHeight();
```
返回当前表面尺寸

## 与 Android 的对比

### 相似之处

1. **线程模型**: 都使用独立的渲染线程
2. **生命周期**: 都由表面创建/销毁触发
3. **主循环结构**: 都是 `while (!exit) { NativeFrame(); }`
4. **初始化顺序**: GL 上下文 → PPSSPP 图形系统 → 主循环

### 差异之处

| 特性 | Android | HarmonyOS |
|------|---------|-----------|
| 表面管理 | ANativeWindow | OH_NativeXComponent |
| 回调注册 | JNI 调用 | OH_NativeXComponent_RegisterCallback |
| 线程启动 | Java 调用 JNI | XComponent 回调触发 |
| 帧率控制 | 依赖 VSync | 手动 sleep 控制 |

## 性能优化

### 1. 帧率控制策略

**当前实现**: 固定 60 FPS 目标
```cpp
const double targetFrameTime = 1.0 / 60.0;
```

**可选优化**:
- 动态调整目标帧率（根据游戏需求）
- 使用 VSync 同步（如果鸿蒙支持）
- 自适应帧率（根据负载）

### 2. 休眠精度

**当前实现**: 使用 `std::this_thread::sleep_for`

**注意事项**:
- 休眠精度取决于系统调度器
- 最小休眠时间约 1ms
- 过短的休眠可能不准确

**可选优化**:
- 使用忙等待（busy-wait）提高精度
- 混合策略：长时间休眠 + 短时间忙等待

### 3. 帧统计开销

**当前实现**: 使用原子操作

**性能影响**: 极小（每帧几纳秒）

**优化建议**:
- 可以在 Release 版本中禁用详细统计
- 只保留必要的 FPS 计数

## 调试和监控

### 日志输出

**FPS 日志**:
```
INFO: FPS: 60.0
```
- 每 5 秒输出一次
- 可用于性能监控

**生命周期日志**:
```
INFO: Starting render loop
INFO: Render thread started
INFO: Entering main render loop
INFO: Exiting render loop
INFO: Render thread ended
```

### 性能分析

**查看 FPS**:
```bash
hdc shell hilog | grep "FPS:"
```

**查看渲染线程状态**:
```bash
hdc shell hilog | grep "Render"
```

**查看帧时间**:
```cpp
// 在代码中添加
INFO_LOG(Log::G3D, "Frame time: %.2f ms", GetLastFrameTime() * 1000.0);
```

## 故障排查

### 问题 1: 渲染循环不启动

**症状**: 黑屏，无日志输出

**检查**:
1. XComponent 是否正确初始化
2. 表面是否创建成功
3. GL 上下文是否初始化

**解决**:
```bash
hdc shell hilog | grep "XComponent"
hdc shell hilog | grep "GL"
```

### 问题 2: FPS 过低

**症状**: FPS < 30

**可能原因**:
1. 渲染负载过高
2. GL 驱动问题
3. 线程调度问题

**调试**:
```cpp
// 添加详细的帧时间日志
INFO_LOG(Log::G3D, "Frame time: %.2f ms (target: 16.67 ms)", 
         GetLastFrameTime() * 1000.0);
```

### 问题 3: 渲染循环无法退出

**症状**: 应用关闭时卡住

**检查**:
1. `g_exitRenderLoop` 是否设置
2. 渲染线程是否阻塞
3. 是否有死锁

**解决**:
```cpp
// 添加超时机制
if (g_renderThread.joinable()) {
    g_exitRenderLoop.store(true);
    // 等待最多 5 秒
    auto future = std::async(std::launch::async, [&]() {
        g_renderThread.join();
    });
    if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
        ERROR_LOG(Log::G3D, "Render thread join timeout!");
    }
}
```

## 未来改进

### 短期
1. ✅ 基本渲染循环
2. ✅ 帧率控制
3. ✅ 帧统计
4. ⏳ VSync 支持（如果鸿蒙提供 API）
5. ⏳ 动态帧率调整

### 中期
1. ⏳ 性能分析工具集成
2. ⏳ 帧时间图表
3. ⏳ GPU 性能监控
4. ⏳ 自适应质量设置

### 长期
1. ⏳ 多线程渲染优化
2. ⏳ 异步资源加载
3. ⏳ 渲染管线优化
4. ⏳ 能耗优化

## 参考

### Android 实现
- `android/jni/app-android.cpp` - 主渲染循环
- `android/jni/AndroidJavaGLContext.cpp` - GL 上下文管理

### PPSSPP 核心
- `Common/System/NativeApp.h` - NativeFrame, NativeInitGraphics
- `Common/GraphicsContext.h` - GraphicsContext 接口
- `Common/TimeUtil.h` - time_now_d()

### 鸿蒙 API
- `ace/xcomponent/native_interface_xcomponent.h` - XComponent 接口
- OpenGL ES 3.0 - 渲染 API

## 总结

渲染循环实现已完成，包括：

- ✅ 独立渲染线程
- ✅ 完整的生命周期管理
- ✅ 60 FPS 帧率控制
- ✅ 实时帧统计
- ✅ 性能监控日志
- ✅ 线程安全设计

**当前状态**: 可以进行编译和测试

**下一步**: 实现触摸输入处理
