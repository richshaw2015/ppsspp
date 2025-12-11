# UI 库集成说明

## 概述

本文档说明 PPSSPP UI 库在鸿蒙版本中的集成方式。

## 集成日期
2024-12-08

## UI 库架构

### 层次结构

```
UI 库
├── 核心应用
│   └── NativeApp.cpp - 应用主逻辑
├── 主要屏幕
│   ├── MainScreen.cpp - 主菜单
│   ├── EmuScreen.cpp - 模拟器运行屏幕
│   ├── GameScreen.cpp - 游戏列表
│   └── PauseScreen.cpp - 暂停菜单
├── 设置屏幕
│   ├── GameSettingsScreen.cpp - 游戏设置
│   ├── ControlMappingScreen.cpp - 控制映射
│   └── SystemInfoScreen.cpp - 系统信息
├── UI 组件
│   ├── BaseScreens.cpp - 基础屏幕类
│   ├── Theme.cpp - 主题系统
│   └── UIAtlas.cpp - UI 图集
└── 调试工具
    └── ImDebugger/ - ImGui 调试器
```

## 集成的文件

### 核心文件（必需）

1. **NativeApp.cpp** - 应用核心
   - `NativeInit()` - 初始化
   - `NativeInitGraphics()` - 图形初始化
   - `NativeFrame()` - 每帧更新
   - `NativeShutdown()` - 清理

2. **EmuScreen.cpp** - 模拟器屏幕
   - 游戏运行时的主屏幕
   - 处理游戏渲染和输入

3. **MainScreen.cpp** - 主菜单
   - 应用启动后的第一个屏幕
   - 游戏列表和设置入口

### 设置屏幕（50+ 文件）

参考 `ui_sources.cmake` 中的完整列表。

## 构建配置

### CMakeLists.txt 配置

```cmake
# 1. 包含 UI 源文件列表
include(${CMAKE_CURRENT_SOURCE_DIR}/ui_sources.cmake)

# 2. 创建 UI 静态库
add_library(UI STATIC ${UI_SOURCES})

# 3. 设置包含目录
target_include_directories(UI PUBLIC
    ${PPSSPP_ROOT}
    ${PPSSPP_ROOT}/Common
    ${PPSSPP_ROOT}/Core
    ${PPSSPP_ROOT}/UI
    ${PPSSPP_ROOT}/ext
)

# 4. 链接依赖
target_link_libraries(UI PUBLIC
    native
    png17_static
    zlibstatic
)

# 5. 使用 whole-archive 链接到主库
target_link_libraries(ppsspp_ohos
    PRIVATE
    -Wl,--whole-archive
    Common
    Core
    UI      # UI 库
    -Wl,--no-whole-archive
)
```

## 与 Android 的对比

### 相同点

1. **文件列表** - 使用相同的 UI 源文件
2. **架构** - 相同的屏幕和组件结构
3. **NativeApp 接口** - 相同的应用生命周期

### 不同点

| 特性 | Android | HarmonyOS |
|------|---------|-----------|
| 平台入口 | app-android.cpp | ohos_xcomponent.cpp |
| 图形上下文 | AndroidJavaGLContext | OhosGLContext |
| 输入处理 | JNI 回调 | XComponent 回调 |
| 资源加载 | AssetManager | rawfile API |

## NativeApp 生命周期

### 初始化流程

```
1. XComponent.onLoad
   ↓
2. OhosXComponent::Initialize()
   ↓
3. OnSurfaceCreated()
   ↓
4. 启动渲染线程
   ↓
5. NativeInit()
   - 初始化配置
   - 加载资源
   - 创建 UI 系统
   ↓
6. NativeInitGraphics(graphicsContext)
   - 初始化图形系统
   - 创建 UI 渲染器
   - 加载纹理和着色器
   ↓
7. 渲染循环
   while (running) {
     NativeFrame(graphicsContext)
     - 更新 UI
     - 处理输入
     - 渲染画面
   }
```

### 清理流程

```
1. 退出渲染循环
   ↓
2. NativeShutdownGraphics()
   - 清理图形资源
   - 销毁 UI 渲染器
   ↓
3. NativeShutdown()
   - 保存配置
   - 清理 UI 系统
   - 释放资源
```

## 关键函数说明

### NativeInit()

```cpp
void NativeInit(int argc, const char *argv[], 
                const char *savegame_dir, 
                const char *external_dir, 
                const char *cache_dir);
```

**功能**:
- 初始化日志系统
- 加载配置文件
- 初始化 UI 系统
- 设置资源路径

**鸿蒙适配**:
- `savegame_dir` - 使用应用数据目录
- `external_dir` - 使用外部存储目录
- `cache_dir` - 使用缓存目录

### NativeInitGraphics()

```cpp
bool NativeInitGraphics(GraphicsContext *graphicsContext);
```

**功能**:
- 初始化 Draw 系统
- 创建 UI 渲染器
- 加载 UI 纹理图集
- 编译 UI 着色器

**返回值**:
- `true` - 初始化成功
- `false` - 初始化失败

### NativeFrame()

```cpp
void NativeFrame(GraphicsContext *graphicsContext);
```

**功能**:
- 更新游戏逻辑
- 处理输入事件
- 更新 UI 状态
- 提交渲染命令

**调用频率**: 每帧一次（通常 60 FPS）

### NativeShutdownGraphics()

```cpp
void NativeShutdownGraphics();
```

**功能**:
- 清理图形资源
- 销毁 UI 渲染器
- 释放纹理和着色器

## UI 系统依赖

### 必需的库

1. **Common** - 基础工具和系统接口
2. **Core** - PPSSPP 核心功能
3. **native** - 图像处理（jpge/jpgd）
4. **png17_static** - PNG 图像加载
5. **zlibstatic** - 压缩支持

### 可选的库

1. **Discord** - Discord 集成（可禁用）
2. **RetroAchievements** - 成就系统（可禁用）
3. **IAP** - 应用内购买（鸿蒙不需要）

## 资源文件

### UI 纹理图集

UI 使用纹理图集来优化渲染：

```
assets/
├── ui_atlas.zim - UI 图集（压缩）
├── ui_atlas.meta - 图集元数据
└── lang/ - 语言文件
    ├── en_US.ini
    ├── zh_CN.ini
    └── ...
```

### 字体文件

```
assets/
└── fonts/
    ├── Roboto-Condensed.ttf
    └── DroidSansFallback.ttf
```

## 编译验证

### 检查 UI 库

```bash
# 查看 UI 库大小
ls -lh entry/.cxx/default/default/debug/arm64-v8a/libUI.a

# 检查 UI 符号
nm libUI.a | grep NativeInit
nm libUI.a | grep NativeFrame
```

### 检查最终库

```bash
# 查看最终库大小
ls -lh entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so

# 检查 UI 符号是否链接
nm -D libppsspp_ohos.so | grep NativeInit
nm -D libppsspp_ohos.so | grep NativeFrame
```

## 常见问题

### 1. 链接错误：undefined reference to NativeInit

**原因**: UI 库未正确链接

**解决**:
```cmake
target_link_libraries(ppsspp_ohos
    PRIVATE
    -Wl,--whole-archive
    UI  # 确保添加了 UI 库
    -Wl,--no-whole-archive
)
```

### 2. 运行时崩溃：找不到资源文件

**原因**: 资源路径未正确设置

**解决**: 在 `NativeInit()` 中设置正确的资源路径

### 3. UI 不显示

**原因**: 
- 图形上下文未正确初始化
- UI 渲染器创建失败
- 纹理加载失败

**调试**: 查看日志中的 `NativeInitGraphics` 输出

## 下一步工作

### 短期

1. ✅ 集成 UI 库源文件
2. ⏳ 编译验证
3. ⏳ 测试基本 UI 显示
4. ⏳ 实现资源加载

### 中期

1. 实现触摸输入处理
2. 适配鸿蒙的文件系统
3. 实现保存/加载功能
4. 添加设置持久化

### 长期

1. 优化 UI 性能
2. 适配不同屏幕尺寸
3. 添加鸿蒙特有功能
4. 完善本地化

## 参考

### PPSSPP 源码
- `UI/NativeApp.cpp` - 应用核心实现
- `UI/EmuScreen.cpp` - 模拟器屏幕实现
- `android/jni/Android.mk` - Android 构建配置

### HarmonyOS 文档
- [资源管理](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/resource-categories-and-access-0000001544463977-V3)
- [文件管理](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/file-management-0000001427584620-V3)

## 总结

UI 库集成完成后，PPSSPP 鸿蒙版将具备完整的用户界面功能，包括：

- ✅ 游戏列表和浏览
- ✅ 游戏运行和暂停
- ✅ 设置和配置
- ✅ 调试工具
- ✅ 在线功能（商店、成就等）

下一步需要编译验证并在设备上测试 UI 显示和交互。
