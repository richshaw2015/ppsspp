# OHOS 平台适配检查清单

## 概述
本文档列出了 PPSSPP 代码库中需要为 OHOS 平台添加适配的关键位置。

## ✅ 已完成的适配

### Core/MemMap.cpp
- **行 217**: 内存映射探测 - 已添加 OHOS 排除
```cpp
#if !PPSSPP_PLATFORM(ANDROID) && !PPSSPP_PLATFORM(OHOS)
```

## 🔍 需要检查的关键位置

### 1. Core 目录

#### Core/Instance.cpp
- **行 21, 84, 145, 182**: 共享内存实现
  - 当前: `PPSSPP_PLATFORM(ANDROID)` 返回 false（不支持）
  - 建议: OHOS 可能也需要类似处理
```cpp
#elif PPSSPP_PLATFORM(ANDROID) || defined(__LIBRETRO__) || PPSSPP_PLATFORM(SWITCH)
// 建议添加: || PPSSPP_PLATFORM(OHOS)
```

#### Core/Config.cpp
- **行 144**: 深度光栅化默认设置
- **行 197**: 状态撤销默认设置
- **行 217**: 屏幕旋转默认设置
- **行 314**: 屏幕旋转配置
- **行 424**: 快进模式默认设置
- **行 432**: 硬件缩放设置
- **行 478**: GPU 后端默认选择
- **行 561**: GPU 后端失败回退
- **行 676**: 自定义驱动配置（ARM64）
- **行 1050**: OSK 键盘绕过设置

**建议**: 大部分移动平台设置应该适用于 OHOS

#### Core/KeyMap.cpp
- **行 106**: DPAD 硬编码
- **行 728**: 默认按键映射
- **行 853**: 手柄自动配置

**建议**: OHOS 应该使用类似 Android 的默认配置

#### Core/Reporting.cpp
- **行 314**: 平台标识符
```cpp
#if defined(__ANDROID__)
    return "Android";
// 建议添加:
#elif PPSSPP_PLATFORM(OHOS)
    return "HarmonyOS";
```

#### Core/FileLoaders/LocalFileLoader.cpp
- **行 26, 46, 74, 189**: 文件加载器实现
  - Content URI 支持
  - 64位文件偏移

**建议**: OHOS 可能需要类似的处理

#### Core/FileSystems/DirectoryFileSystem.cpp
- **行 54**: 文件系统头文件
```cpp
#if defined(__ANDROID__)
#include <sys/types.h>
#include <sys/vfs.h>
#define statvfs statfs
// 建议添加 OHOS 检查
```

### 2. Common 目录

#### Common/Thread/ThreadUtil.cpp
- **行 9, 34, 52, 142, 230**: 线程工具
  - JNI 附加/分离
  - 线程命名
  - 线程 ID 获取

**建议**: OHOS 应该使用 Linux 风格的实现

#### Common/MemArenaPosix.cpp
- **行 20**: 平台排除
```cpp
#if !defined(_WIN32) && !defined(ANDROID) && !defined(__APPLE__) && !PPSSPP_PLATFORM(SWITCH)
// 建议添加: && !PPSSPP_PLATFORM(OHOS)
```

#### Common/Log/LogManager.cpp
- **行 20, 60, 386**: Android 日志实现
**建议**: OHOS 已有 HiLog 实现，无需修改

#### Common/CPUDetect.cpp
- **行 28**: CPU 检测
```cpp
#ifdef __ANDROID__
#include <sys/stat.h>
#include <fcntl.h>
// 可能需要 OHOS 特定处理
```

#### Common/TimeUtil.cpp
- **行 121**: 时间工具
```cpp
#elif PPSSPP_PLATFORM(ANDROID) || PPSSPP_PLATFORM(LINUX) || ...
// OHOS 已包含在 LINUX 中
```

#### Common/MemoryUtil.cpp
- **行 253**: 内存对齐分配
```cpp
#ifdef __ANDROID__
    ptr = memalign(alignment, size);
// 建议添加 OHOS 检查
```

#### Common/Render/Text/draw_text.cpp
- **行 265**: 文本渲染器选择
```cpp
#elif PPSSPP_PLATFORM(ANDROID)
    drawer = new TextDrawerAndroid(draw);
// 建议: OHOS 可能需要自己的实现或使用 Android 的
```

#### Common/GPU/OpenGL/GLFeatures.cpp
- **行 21, 406, 459**: OpenGL 扩展检测
**建议**: OHOS 可能需要类似的特殊处理

#### Common/GPU/OpenGL/thin3d_gl.cpp
- **行 693**: Qualcomm GPU bug 检测
```cpp
#if PPSSPP_PLATFORM(ANDROID)
// 建议添加 OHOS 检查
```

### 3. UI 目录

#### UI/DisplayLayoutScreen.cpp
- **行 277**: 安全区域支持检测
```cpp
#if PPSSPP_PLATFORM(ANDROID)
    supportsInsets = System_GetPropertyInt(SYSPROP_SYSTEMVERSION) >= 28;
// 建议添加:
#elif PPSSPP_PLATFORM(OHOS)
    supportsInsets = true;  // OHOS 支持安全区域
```

#### UI/MemStickScreen.cpp
- **行 67, 309**: Memory Stick 文件夹选择
**建议**: OHOS 应该使用文件选择器

#### UI/DiscordIntegration.cpp
- **行 12**: Discord 集成
```cpp
#if (PPSSPP_PLATFORM(WINDOWS) || PPSSPP_PLATFORM(MAC) || PPSSPP_PLATFORM(LINUX)) && !PPSSPP_PLATFORM(ANDROID) && !PPSSPP_PLATFORM(UWP)
// 建议添加: && !PPSSPP_PLATFORM(OHOS)
```

#### UI/UIAtlas.cpp
- **行 496**: 字体纹理加载
```cpp
#if PPSSPP_PLATFORM(WINDOWS) || PPSSPP_PLATFORM(ANDROID) || PPSSPP_PLATFORM(MAC) || PPSSPP_PLATFORM(IOS)
// 建议添加: || PPSSPP_PLATFORM(OHOS)
```

#### UI/EmuScreen.cpp
- **行 409**: 省电模式警告
```cpp
#ifdef __ANDROID__
    g_OSD.Show(OSDType::MESSAGE_WARNING, sy->T("WARNING: Android battery save mode is on"), 2.0f, "core_powerSaving");
// 建议添加 OHOS 检查
```

#### UI/MiscScreens.cpp
- **行 288**: 阿拉伯语支持
- **行 700**: 关于页面工具列表

#### UI/IAPScreen.cpp
- **行 67, 89**: 应用内购买
**建议**: OHOS 需要自己的应用商店集成

#### UI/MainScreen.cpp
- **行 663**: 主目录路径
- **行 1221**: 退出按钮
```cpp
#if !PPSSPP_PLATFORM(IOS_APP_STORE) && !PPSSPP_PLATFORM(ANDROID)
// 建议添加: && !PPSSPP_PLATFORM(OHOS)
```

#### UI/NativeApp.cpp
- **行 148, 152, 217, 274, 441, 675, 686, 711, 1230, 1305**: 多处 Android 特定代码
**建议**: 大部分需要 OHOS 适配

## 📋 优先级建议

### 高优先级（影响核心功能）
1. ✅ **Core/MemMap.cpp** - 已修复
2. **Core/Config.cpp** - GPU 后端、屏幕旋转等默认设置
3. **Core/Reporting.cpp** - 平台标识
4. **Common/Thread/ThreadUtil.cpp** - 线程管理
5. **UI/NativeApp.cpp** - 应用初始化

### 中优先级（影响用户体验）
1. **UI/DisplayLayoutScreen.cpp** - 安全区域
2. **UI/MemStickScreen.cpp** - 文件管理
3. **Core/KeyMap.cpp** - 输入映射
4. **Common/Render/Text/draw_text.cpp** - 文本渲染

### 低优先级（可选功能）
1. **UI/DiscordIntegration.cpp** - Discord 集成
2. **UI/IAPScreen.cpp** - 应用内购买
3. **Core/Instance.cpp** - 多实例支持

## 🔧 推荐的适配策略

### 1. 使用宏定义模式
```cpp
#if PPSSPP_PLATFORM(ANDROID) || PPSSPP_PLATFORM(OHOS)
    // 移动平台通用代码
#endif
```

### 2. OHOS 特定代码
```cpp
#if PPSSPP_PLATFORM(OHOS)
    // OHOS 专用实现
#elif PPSSPP_PLATFORM(ANDROID)
    // Android 实现
#endif
```

### 3. 排除桌面平台
```cpp
#if !PPSSPP_PLATFORM(ANDROID) && !PPSSPP_PLATFORM(OHOS)
    // 桌面平台代码
#endif
```

## 📝 注意事项

1. **不要盲目复制 Android 代码** - OHOS 有自己的 API
2. **优先使用 OHOS 原生 API** - 如 HiLog、文件选择器等
3. **测试每个修改** - 确保不影响其他平台
4. **保持代码整洁** - 使用清晰的注释说明平台差异

## 🎯 下一步行动

1. 逐个检查上述位置
2. 根据实际需求决定是否需要适配
3. 编写测试用例验证功能
4. 更新文档记录所有修改
