# PPSSPP 鸿蒙版 - OpenGL ES 渲染管线

## 📋 概述

本文档是 PPSSPP 鸿蒙版 OpenGL ES 渲染管线实现的入口文档。

**实现日期**: 2024-12-08  
**状态**: ✅ 实现完成，待编译验证  
**参考**: Android 版本 (android/jni/AndroidJavaGLContext.cpp)

## 🎯 实现内容

### 核心功能

✅ **EGL 上下文管理** - 初始化、配置、销毁  
✅ **Draw 系统集成** - PPSSPP thin3d 抽象层  
✅ **GLRenderManager** - OpenGL 渲染管理器  
✅ **XComponent 集成** - 鸿蒙原生渲染组件  
✅ **渲染线程** - 独立渲染线程和循环  
✅ **NAPI 绑定** - ArkTS 接口导出  

### 新增文件

```
ohos/entry/src/main/cpp/
├── ohos_graphics_context.h      # 图形上下文基类（更新）
├── ohos_gl_context.h            # OpenGL 上下文头文件（更新）
├── ohos_gl_context.cpp          # OpenGL 上下文实现（更新）
├── ohos_xcomponent.h            # XComponent 集成头文件（新增）
├── ohos_xcomponent.cpp          # XComponent 集成实现（新增）
└── napi/napi_init.cpp           # NAPI 绑定（更新）

ohos/docs/
├── OPENGL_IMPLEMENTATION.md     # 详细实现文档
├── OPENGL_IMPLEMENTATION_SUMMARY.md  # 实现总结
└── XCOMPONENT_USAGE.md          # ArkTS 使用指南

ohos/
├── OPENGL_README.md             # 本文件
├── OPENGL_QUICKSTART.md         # 快速开始指南
└── OPENGL_CHECKLIST.md          # 实现检查清单
```

## 📚 文档导航

### 快速开始

- **[OPENGL_QUICKSTART.md](OPENGL_QUICKSTART.md)** - 快速开始指南
  - 编译步骤
  - 最小测试代码
  - 运行测试
  - 故障排查

### 开发文档

- **[docs/OPENGL_IMPLEMENTATION.md](docs/OPENGL_IMPLEMENTATION.md)** - 详细实现文档
  - 架构设计
  - 渲染流程
  - 与 Android 对比
  - EGL 配置
  - 性能优化
  - 调试指南

- **[docs/XCOMPONENT_USAGE.md](docs/XCOMPONENT_USAGE.md)** - ArkTS 使用指南
  - 基本用法
  - 完整示例
  - 生命周期管理
  - 配置管理
  - 错误处理

### 项目管理

- **[docs/OPENGL_IMPLEMENTATION_SUMMARY.md](docs/OPENGL_IMPLEMENTATION_SUMMARY.md)** - 实现总结
  - 完成内容列表
  - 架构设计
  - 关键技术点
  - 待实现功能

- **[OPENGL_CHECKLIST.md](OPENGL_CHECKLIST.md)** - 实现检查清单
  - 代码实现检查
  - 文档完整性
  - 测试计划
  - 验证步骤

## 🚀 快速开始

### 1. 编译

```bash
cd ohos
./gradlew clean
./gradlew assembleDebug
```

### 2. 检查产物

```bash
ls -lh entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so
```

### 3. 最小测试代码

```typescript
import ppsspp from 'libppsspp_ohos.so'

@Entry
@Component
struct TestGL {
  private xComponentController: XComponentController = new XComponentController()
  
  build() {
    XComponent({
      id: 'test_surface',
      type: 'surface',
      controller: this.xComponentController
    })
    .onLoad(() => {
      ppsspp.initXComponent(this.xComponentController)
      ppsspp.initEmulator()
    })
    .width('100%')
    .height('100%')
  }
}
```

详细步骤请参考 [OPENGL_QUICKSTART.md](OPENGL_QUICKSTART.md)

## 🏗️ 架构概览

### 层次结构

```
ArkTS 层 (TypeScript)
    ↓ NAPI
Native 层 (C++)
    ↓
XComponent 集成层 (ohos_xcomponent.cpp)
    ↓
OpenGL 上下文层 (ohos_gl_context.cpp)
    ↓
EGL 层
    ↓
OpenGL ES 3.0
```

### 渲染流程

```
1. XComponent.onLoad
   ↓
2. initXComponent() [NAPI]
   ↓
3. OhosXComponent::Initialize()
   ↓
4. OnSurfaceCreated()
   ↓
5. 启动渲染线程
   ↓
6. InitFromRenderThread()
   - 初始化 EGL
   - 创建 Draw::DrawContext
   - 获取 GLRenderManager
   ↓
7. 渲染循环
   while (running) {
     NativeFrame()
     ThreadFrame()
   }
```

详细架构请参考 [docs/OPENGL_IMPLEMENTATION.md](docs/OPENGL_IMPLEMENTATION.md)

## 🔍 关键技术点

### EGL 配置

```cpp
EGL_SURFACE_TYPE: EGL_WINDOW_BIT
EGL_RENDERABLE_TYPE: EGL_OPENGL_ES3_BIT
EGL_RED_SIZE: 8, GREEN_SIZE: 8, BLUE_SIZE: 8, ALPHA_SIZE: 8
EGL_DEPTH_SIZE: 24, STENCIL_SIZE: 8
```

### Draw 上下文创建

```cpp
draw_ = Draw::T3DCreateGLContext(false);
renderManager_ = (GLRenderManager*)draw_->GetNativeObject(
    Draw::NativeObject::RENDER_MANAGER);
```

### 渲染循环

```cpp
while (!exitRenderLoop) {
    NativeFrame(graphicsContext);
    graphicsContext->ThreadFrame(true);
}
```

## 📊 与 Android 对比

| 特性 | Android | HarmonyOS |
|------|---------|-----------|
| 表面组件 | SurfaceView | XComponent |
| 窗口类型 | ANativeWindow | OHNativeWindow |
| 回调机制 | JNI 调用 | XComponent 回调 |
| 线程管理 | Java Thread | std::thread |
| 架构 | ✅ 相同 | ✅ 相同 |
| Draw 系统 | ✅ 相同 | ✅ 相同 |
| GLRenderManager | ✅ 相同 | ✅ 相同 |

## ✅ 已实现功能

- [x] EGL 上下文初始化和管理
- [x] Draw::DrawContext 创建
- [x] GLRenderManager 集成
- [x] XComponent 回调处理
- [x] 渲染线程管理
- [x] 渲染循环实现
- [x] 表面生命周期管理
- [x] NAPI 绑定
- [x] 与 PPSSPP Core 集成

## 🔜 待实现功能

### 高优先级

- [ ] 触摸输入处理
- [ ] VSync 同步优化
- [ ] 错误恢复机制

### 中优先级

- [ ] 性能监控和统计
- [ ] 配置管理
- [ ] 调试工具

### 低优先级

- [ ] Vulkan 支持
- [ ] 多窗口支持

## 🐛 故障排查

### 常见问题

1. **XComponent 初始化失败**
   ```bash
   # 检查符号
   nm -D libppsspp_ohos.so | grep Initialize
   ```

2. **EGL 初始化失败**
   ```bash
   # 查看 EGL 错误
   hdc shell hilog | grep EGL
   ```

3. **渲染线程崩溃**
   ```bash
   # 检查 Core 符号
   nm -D libppsspp_ohos.so | grep NativeInitGraphics
   ```

详细故障排查请参考 [OPENGL_QUICKSTART.md](OPENGL_QUICKSTART.md#故障排查)

## 📝 日志标签

- `PPSSPP_GL` - OpenGL 上下文相关
- `PPSSPP_XComponent` - XComponent 集成相关
- `Log::G3D` - PPSSPP 图形系统日志

### 查看日志

```bash
# 查看所有 PPSSPP 日志
hdc shell hilog | grep PPSSPP

# 查看 OpenGL 日志
hdc shell hilog | grep PPSSPP_GL

# 查看 XComponent 日志
hdc shell hilog | grep PPSSPP_XComponent
```

## 🧪 测试

### 编译验证

```bash
cd ohos
./gradlew clean assembleDebug
```

### 符号验证

```bash
nm -D entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep -i "OhosGL"
nm -D entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep -i "XComponent"
```

### 运行验证

```bash
./gradlew installDebug
hdc shell hilog | grep PPSSPP
```

详细测试步骤请参考 [OPENGL_CHECKLIST.md](OPENGL_CHECKLIST.md#测试计划)

## 📖 参考资料

### PPSSPP 源码

- `android/jni/AndroidJavaGLContext.{h,cpp}` - Android OpenGL 实现
- `android/jni/app-android.cpp` - Android 应用集成
- `Common/GraphicsContext.h` - 图形上下文基类
- `Common/GPU/OpenGL/GLRenderManager.h` - OpenGL 渲染管理器
- `Common/GPU/thin3d_create.h` - Draw 上下文创建

### HarmonyOS 文档

- [XComponent 开发指南](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/napi-xcomponent-guidelines-0000001281201106-V3)
- [Native API 参考](https://developer.harmonyos.com/cn/docs/documentation/doc-references-V3/native-apis-0000001281201122-V3)

### OpenGL 文档

- [EGL API](https://www.khronos.org/registry/EGL/sdk/docs/man/)
- [OpenGL ES 3.0](https://www.khronos.org/registry/OpenGL/index_es.php)

## 🤝 贡献

本实现参考了 Android 版本的成熟架构，代码质量较高，结构清晰。欢迎贡献：

1. 完善触摸输入处理
2. 优化性能
3. 添加测试用例
4. 改进文档

## 📄 许可证

遵循 PPSSPP 项目的许可证（GPLv2+）

## 📞 联系方式

如有问题，请参考：
- [OPENGL_QUICKSTART.md](OPENGL_QUICKSTART.md) - 快速开始
- [docs/OPENGL_IMPLEMENTATION.md](docs/OPENGL_IMPLEMENTATION.md) - 详细文档
- [OPENGL_CHECKLIST.md](OPENGL_CHECKLIST.md) - 检查清单

---

**最后更新**: 2024-12-08  
**版本**: 1.0.0  
**状态**: ✅ 实现完成，待编译验证
