# OpenGL ES 渲染管线 - 快速开始

## 概述

本指南帮助你快速测试 PPSSPP 鸿蒙版的 OpenGL ES 渲染管线。

## 前置条件

- DevEco Studio 4.0+
- HarmonyOS SDK API 10+
- 鸿蒙设备或模拟器

## 编译步骤

### 1. 清理构建

```bash
cd ohos
./gradlew clean
```

### 2. 编译 Native 库

```bash
./gradlew assembleDebug
```

### 3. 检查产物

```bash
# 检查库文件大小
ls -lh entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so

# 检查 OpenGL 相关符号
nm -D entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep -i "OhosGL"
nm -D entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep -i "XComponent"

# 检查 PPSSPP Core 符号
nm -D entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep -i "NativeInit"
```

## 最小测试代码

### 1. 创建测试页面

在 `ohos/entry/src/main/ets/pages/TestGL.ets`:

```typescript
import ppsspp from 'libppsspp_ohos.so'

@Entry
@Component
struct TestGL {
  private xComponentController: XComponentController = new XComponentController()
  @State status: string = 'Not initialized'
  
  build() {
    Column() {
      Text(this.status)
        .fontSize(16)
        .margin(10)
      
      XComponent({
        id: 'test_surface',
        type: 'surface',
        controller: this.xComponentController
      })
      .onLoad(() => {
        console.info('[TestGL] XComponent loaded')
        this.status = 'XComponent loaded'
        
        try {
          // 初始化 XComponent
          let result = ppsspp.initXComponent(this.xComponentController)
          if (result) {
            console.info('[TestGL] XComponent initialized')
            this.status = 'XComponent initialized'
            
            // 初始化模拟器
            result = ppsspp.initEmulator()
            if (result) {
              console.info('[TestGL] Emulator initialized')
              this.status = 'Emulator initialized - Ready!'
            } else {
              console.error('[TestGL] Failed to initialize emulator')
              this.status = 'Error: Failed to initialize emulator'
            }
          } else {
            console.error('[TestGL] Failed to initialize XComponent')
            this.status = 'Error: Failed to initialize XComponent'
          }
        } catch (error) {
          console.error('[TestGL] Exception: ' + error)
          this.status = 'Error: ' + error
        }
      })
      .onDestroy(() => {
        console.info('[TestGL] XComponent destroyed')
        try {
          ppsspp.shutdownEmulator()
        } catch (error) {
          console.error('[TestGL] Shutdown error: ' + error)
        }
      })
      .width('100%')
      .height('80%')
    }
    .width('100%')
    .height('100%')
  }
}
```

### 2. 更新路由配置

在 `ohos/entry/src/main/resources/base/profile/main_pages.json`:

```json
{
  "src": [
    "pages/Index",
    "pages/TestGL"
  ]
}
```

### 3. 添加导航按钮

在 `ohos/entry/src/main/ets/pages/Index.ets` 添加测试按钮:

```typescript
Button('Test OpenGL')
  .onClick(() => {
    router.pushUrl({ url: 'pages/TestGL' })
  })
```

## 运行测试

### 1. 连接设备

```bash
# 查看设备
hdc list targets

# 如果没有设备，启动模拟器
# 在 DevEco Studio 中: Tools -> Device Manager -> 启动模拟器
```

### 2. 安装应用

在 DevEco Studio 中点击 Run 按钮，或使用命令行：

```bash
./gradlew installDebug
```

### 3. 查看日志

```bash
# 查看所有 PPSSPP 日志
hdc shell hilog | grep PPSSPP

# 查看 OpenGL 相关日志
hdc shell hilog | grep PPSSPP_GL

# 查看 XComponent 相关日志
hdc shell hilog | grep PPSSPP_XComponent

# 查看测试页面日志
hdc shell hilog | grep TestGL
```

## 预期结果

### 成功的日志输出

```
[TestGL] XComponent loaded
[PPSSPP_XComponent] Initializing XComponent
[PPSSPP_XComponent] XComponent initialized
[PPSSPP_XComponent] XComponent surface created
[PPSSPP_XComponent] Surface size: 1080x1920
[PPSSPP_XComponent] Starting render loop
[PPSSPP_XComponent] Render thread started
[PPSSPP_GL] OhosGLContext::InitFromRenderThread: desiredWidth=1080 desiredHeight=1920
[PPSSPP_GL] EGL version: 1.4
[PPSSPP_GL] GL Extensions: ...
[PPSSPP_GL] OhosGLContext initialized successfully
[TestGL] Emulator initialized - Ready!
```

### 界面显示

- 顶部显示状态文本: "Emulator initialized - Ready!"
- 下方显示黑色渲染表面（XComponent）
- 没有崩溃或错误提示

## 故障排查

### 问题 1: XComponent 初始化失败

**症状**: 日志显示 "Failed to initialize XComponent"

**可能原因**:
- XComponent 回调注册失败
- Native 窗口获取失败

**解决方法**:
```bash
# 检查 XComponent 符号是否存在
nm -D entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep Initialize
```

### 问题 2: EGL 初始化失败

**症状**: 日志显示 "Failed to initialize EGL"

**可能原因**:
- EGL 配置不支持
- 设备不支持 OpenGL ES 3.0

**解决方法**:
```bash
# 查看详细的 EGL 错误
hdc shell hilog -b D | grep EGL
```

### 问题 3: 渲染线程崩溃

**症状**: 应用启动后立即崩溃

**可能原因**:
- 缺少必要的库
- 符号未正确链接

**解决方法**:
```bash
# 检查库依赖
readelf -d entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep NEEDED

# 检查 Core 符号
nm -D entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep NativeInitGraphics
```

### 问题 4: 编译错误

**症状**: 编译失败，找不到头文件

**可能原因**:
- 包含目录配置错误
- 缺少依赖库

**解决方法**:
```bash
# 检查 CMakeLists.txt 配置
cat ohos/entry/src/main/cpp/CMakeLists.txt | grep include_directories

# 重新生成构建文件
./gradlew clean
./gradlew assembleDebug
```

## 下一步测试

### 1. 测试表面生命周期

```typescript
// 添加按钮测试暂停/恢复
Button('Pause')
  .onClick(() => {
    ppsspp.pauseEmulator()
  })

Button('Resume')
  .onClick(() => {
    ppsspp.resumeEmulator()
  })
```

### 2. 测试表面尺寸变化

```typescript
// 旋转屏幕或改变窗口大小
// 观察日志中的 "Surface changed" 消息
```

### 3. 测试加载游戏

```typescript
Button('Load Test Game')
  .onClick(() => {
    // 需要先准备一个测试 ROM
    let result = ppsspp.loadGame('/data/storage/el2/base/files/test.iso')
    console.info('Load game result: ' + result)
  })
```

## 性能测试

### 1. 帧率测试

添加帧率显示：

```typescript
@State fps: number = 0
private frameCount: number = 0
private lastTime: number = Date.now()

// 在渲染循环中更新
private updateFPS() {
  this.frameCount++
  let now = Date.now()
  if (now - this.lastTime >= 1000) {
    this.fps = this.frameCount
    this.frameCount = 0
    this.lastTime = now
  }
}
```

### 2. 内存监控

```bash
# 监控内存使用
hdc shell hidumper -s MemoryManagerService -a "-p <pid>"
```

### 3. CPU 使用

```bash
# 监控 CPU 使用
hdc shell top | grep ppsspp
```

## 成功标准

✅ 编译成功，生成 libppsspp_ohos.so
✅ XComponent 初始化成功
✅ EGL 上下文创建成功
✅ 渲染线程启动成功
✅ 没有崩溃或错误
✅ 日志显示正常的初始化流程

## 参考文档

- [OPENGL_IMPLEMENTATION.md](docs/OPENGL_IMPLEMENTATION.md) - 详细实现说明
- [XCOMPONENT_USAGE.md](docs/XCOMPONENT_USAGE.md) - ArkTS 使用指南
- [OPENGL_IMPLEMENTATION_SUMMARY.md](docs/OPENGL_IMPLEMENTATION_SUMMARY.md) - 实现总结

## 获取帮助

如果遇到问题：

1. 查看日志输出
2. 检查符号是否正确链接
3. 参考故障排查部分
4. 查看详细实现文档
