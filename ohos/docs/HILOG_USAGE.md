# OHOS HiLog 使用指南

## 问题背景

OHOS 的 `hilog/log.h` 定义了 `LogLevel` 枚举，与 PPSSPP 的 `Common/Log.h` 中的 `LogLevel` 冲突。直接包含 `hilog/log.h` 会导致编译错误。

## 解决方案

使用 `ohos/entry/src/main/cpp/ohos_hilog.h` 包装器，它声明了 `OH_LOG_Print` 函数而不包含 `hilog/log.h`。

## 使用方法

### 1. 在 OHOS 平台特定的 C++ 文件中

对于 `ohos/entry/src/main/cpp/` 目录下的文件（如 `ohos_xcomponent.cpp`、`napi_init.cpp` 等）：

```cpp
#include "ohos_hilog.h"

#define TAG "YourModuleName"

// 使用日志宏
OHOS_LOGI(TAG, "Info message: %s", someString);
OHOS_LOGW(TAG, "Warning message: %d", someInt);
OHOS_LOGE(TAG, "Error message");
OHOS_LOGD(TAG, "Debug message");
```

### 2. 在跨平台的 C++ 文件中

对于 PPSSPP 核心代码（如 `UI/NativeApp.cpp`、`Core/` 等）：

```cpp
#include "ppsspp_config.h"

#if defined(OHOS) || defined(__OHOS__)
// 包含 OHOS HiLog 包装器
#include "../ohos/entry/src/main/cpp/ohos_hilog.h"
#define OHOS_LOG_TAG "YourModuleName"
#endif

void SomeFunction() {
#if defined(OHOS) || defined(__OHOS__)
    OHOS_LOGI(OHOS_LOG_TAG, "OHOS specific log");
#endif
    
    // 使用 PPSSPP 的日志系统（跨平台）
    INFO_LOG(Log::System, "Cross-platform log");
}
```

## 可用的日志宏

- `OHOS_LOGD(tag, fmt, ...)` - Debug 级别
- `OHOS_LOGI(tag, fmt, ...)` - Info 级别
- `OHOS_LOGW(tag, fmt, ...)` - Warning 级别
- `OHOS_LOGE(tag, fmt, ...)` - Error 级别

## 日志格式

```cpp
// 基本用法
OHOS_LOGI("MyTag", "Simple message");

// 带参数
OHOS_LOGI("MyTag", "Value: %d", 42);
OHOS_LOGI("MyTag", "String: %s", someString);

// 多个参数
OHOS_LOGI("MyTag", "x=%d, y=%d, name=%s", x, y, name);
```

## 注意事项

1. **不要直接包含 `hilog/log.h`**，使用 `ohos_hilog.h` 包装器
2. **TAG 长度限制**：建议不超过 32 个字符
3. **日志域**：已在 `ohos_hilog.h` 中定义为 `0x0001`
4. **条件编译**：在跨平台代码中使用 `#if defined(OHOS)` 保护 OHOS 特定代码
5. **CMake 配置**：确保目标库定义了 `OHOS=1` 宏

## CMake 配置示例

```cmake
# 为库添加 OHOS 宏定义
target_compile_definitions(YourLibrary PRIVATE
    OHOS=1
    MOBILE_DEVICE=1
)
```

## 已应用的文件

- `ohos/entry/src/main/cpp/napi/napi_init.cpp`
- `ohos/entry/src/main/cpp/ohos_xcomponent.cpp`
- `ohos/entry/src/main/cpp/ohos_gl_context.cpp`
- `ohos/entry/src/main/cpp/ohos_system.cpp`
- `UI/NativeApp.cpp`

## 查看日志

在 DevEco Studio 中：
1. 打开 **HiLog** 窗口
2. 过滤 TAG 或应用包名
3. 选择日志级别

命令行查看：
```bash
hdc shell hilog | grep "YourTag"
```
