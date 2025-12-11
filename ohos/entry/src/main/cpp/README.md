# PPSSPP 鸿蒙 Native 代码

## 文件说明

### NAPI 接口层

- **napi/napi_init.cpp**: NAPI 模块初始化和注册
- **napi/napi_ppsspp.cpp**: PPSSPP 功能的 NAPI 导出实现
- **napi/napi_ppsspp.h**: NAPI 接口声明

### 平台适配层

- **ohos_app.cpp/h**: 应用生命周期管理，协调各子系统
- **ohos_system.cpp/h**: 系统接口封装（文件系统、权限等）
- **ohos_audio.cpp/h**: 音频后端实现
- **ohos_gl_context.cpp/h**: OpenGL ES 上下文管理
- **ohos_graphics_context.h**: 图形上下文抽象接口
- **ohos_input.cpp/h**: 输入事件处理（触摸、按键、手柄）

## 编译说明

### 依赖库

鸿蒙系统库：
- `ace_napi.z`: NAPI 接口
- `hilog_ndk.z`: 日志系统
- `EGL`: OpenGL ES 窗口系统
- `GLESv3`: OpenGL ES 3.0
- `native_window`: 原生窗口
- `native_drawing`: 2D 绘图
- `rawfile.z`: 资源文件访问

PPSSPP 库（待集成）：
- `Common`: 通用工具库
- `Core`: 模拟器核心
- 第三方库：snappy, glslang, libzip 等

### 编译选项

```cmake
# 平台宏定义
-DOHOS=1                    # 鸿蒙平台
-DMOBILE_DEVICE=1           # 移动设备
-DUSING_GLES2=1             # 使用 OpenGL ES
-DUSING_EGL=1               # 使用 EGL
-D__STDC_CONSTANT_MACROS    # C++ 常量宏
-DGLEW_NO_GLU               # 不使用 GLU

# 编译器选项
-std=c++17                  # C++17 标准
-Wall                       # 所有警告
-fno-strict-aliasing        # 禁用严格别名
-fno-math-errno             # 数学函数不设置 errno
```

## 开发流程

### 1. 实现新功能

```cpp
// 1. 在对应的 .h 文件中声明接口
namespace OhosXXX {
    bool NewFunction();
}

// 2. 在对应的 .cpp 文件中实现
bool NewFunction() {
    OH_LOG_INFO(LOG_APP, "NewFunction called");
    // 实现逻辑
    return true;
}

// 3. 如需导出到 ArkTS，在 napi_ppsspp.cpp 中添加
napi_value NewFunction(napi_env env, napi_callback_info info) {
    bool result = OhosXXX::NewFunction();
    napi_value napiResult;
    napi_get_boolean(env, result, &napiResult);
    return napiResult;
}

// 4. 在 napi_init.cpp 中注册
{"newFunction", nullptr, NapiPPSSPP::NewFunction, ...}
```

### 2. 调试技巧

```cpp
// 使用 HiLog 输出不同级别的日志
OH_LOG_DEBUG(LOG_APP, "Debug info: %{public}d", value);
OH_LOG_INFO(LOG_APP, "Info message");
OH_LOG_WARN(LOG_APP, "Warning: %{public}s", msg.c_str());
OH_LOG_ERROR(LOG_APP, "Error occurred");
OH_LOG_FATAL(LOG_APP, "Fatal error");

// 注意：使用 %{public} 标记公开信息，否则会被隐私保护
```

### 3. 性能优化

- 使用 NEON 指令集（ARM）
- 避免频繁的 JNI/NAPI 调用
- 使用对象池减少内存分配
- 多线程并行处理

## 集成 PPSSPP Core

### 步骤 1: 添加 ext 依赖

```cmake
# 在 CMakeLists.txt 中添加
add_subdirectory(${PPSSPP_ROOT}/ext ${CMAKE_BINARY_DIR}/ext)
```

### 步骤 2: 编译 Common 库

```cmake
add_subdirectory(${PPSSPP_ROOT}/Common ${CMAKE_BINARY_DIR}/Common)
target_link_libraries(ppsspp_ohos PRIVATE Common)
```

### 步骤 3: 编译 Core 库

```cmake
add_subdirectory(${PPSSPP_ROOT}/Core ${CMAKE_BINARY_DIR}/Core)
target_link_libraries(ppsspp_ohos PRIVATE Core)
```

### 步骤 4: 实现平台接口

参考 `android/jni/app-android.cpp` 实现：
- NativeApp.h 中定义的接口
- System.h 中的系统接口
- GraphicsContext 的具体实现

## 常见问题

### Q: 链接错误 undefined reference

A: 检查库的链接顺序，确保依赖库在被依赖库之前

### Q: 运行时找不到符号

A: 确认 .so 文件已正确打包到 HAP 中

### Q: EGL 初始化失败

A: 检查 XComponent 是否已创建，native_window 是否有效

### Q: 音频播放卡顿

A: 调整缓冲区大小，使用低延迟模式

## 参考代码

- Android 实现: `android/jni/app-android.cpp`
- iOS 实现: `ios/main.mm`
- SDL 实现: `SDL/SDLMain.cpp`
