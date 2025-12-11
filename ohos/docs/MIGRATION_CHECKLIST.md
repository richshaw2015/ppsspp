# PPSSPP OHOS 移植检查清单

## 问题诊断

### 当前问题
- UI 不显示，只显示 "Hello World"
- Index.ets 被重置为默认模板

### 根本原因
1. **Index.ets 被重置** - 需要重新实现 XComponent 集成
2. **NativeInit 未被调用** - PPSSPP 核心未初始化
3. **桩函数覆盖真实实现** - `ohos_native_app_stubs.cpp` 的缓存仍在被使用
4. **构建缓存未清理** - ninja 缓存包含旧的编译结果

## 修复步骤

### 1. 清理构建缓存
```bash
cd ohos
rm -rf entry/.cxx
rm -rf .hvigor
hvigorw clean
```

### 2. 确认 Index.ets 已更新
文件应该包含：
- XComponent 组件
- libraryname: 'ppsspp_ohos'
- initEmulator() 调用

### 3. 确认 napi_init.cpp 已更新
应该包含：
- NativeXComponentInit 函数
- InitEmulator 调用 NativeInit
- XComponent 回调注册

### 4. 确认桩文件已移除
CMakeLists.txt 中 `ohos_native_app_stubs.cpp` 应该被注释掉

### 5. 重新编译
```bash
cd ohos
hvigorw assembleHap
```

## 初始化流程（参考 Android）

```
1. 应用启动
   ↓
2. Index.ets aboutToAppear()
   ↓
3. ppsspp.initEmulator()
   ↓
4. NativeInit() - 初始化 PPSSPP 核心
   ↓
5. XComponent 创建
   ↓
6. NativeXComponentInit() - 注册回调
   ↓
7. OnSurfaceCreated() - Surface 创建
   ↓
8. g_display.Recalculate() - 设置显示参数
   ↓
9. NativeResized() - 初始化 UI 系统
   ↓
10. RenderLoop() - 启动渲染循环
    ↓
11. NativeInitGraphics() - 初始化图形
    ↓
12. NativeFrame() - 渲染每一帧
    ↓
13. UI 显示 ✅
```

## 关键函数

### NativeInit (UI/NativeApp.cpp)
- 初始化 VFS
- 加载配置
- 初始化线程管理器
- 设置路径

### NativeInitGraphics (UI/NativeApp.cpp)
- 创建 Draw::DrawContext
- 初始化 UI 系统
- 创建 EmuScreen

### NativeFrame (UI/NativeApp.cpp)
- 处理消息
- 更新 UI
- 渲染画面

### NativeResized (UI/NativeApp.cpp)
- 更新显示参数
- 重新布局 UI

## 验证步骤

### 1. 查看日志
```bash
hdc shell hilog -x | grep -E "PPSSPP|NativeInit|NativeFrame|XComponent"
```

### 2. 期望看到的日志
```
========== InitEmulator START ==========
Calling NativeInit...
NativeInit completed
========== NativeXComponentInit START ==========
Registering XComponent callbacks...
Callbacks registered successfully
=== OnSurfaceCreated START ===
Setting display params...
Calling NativeResized...
=== Starting Render Loop ===
Calling NativeInitGraphics...
NativeInitGraphics succeeded
```

### 3. 如果仍然失败
检查：
- 是否有链接错误
- 是否有运行时崩溃
- 是否有 OpenGL 错误
