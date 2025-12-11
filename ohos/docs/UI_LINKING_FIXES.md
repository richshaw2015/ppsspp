# UI 库链接错误修复

## 修复日期
2024-12-08

## 问题描述

### 重复符号错误

**错误信息**:
```
ld.lld: error: duplicate symbol: System_PostUIMessage(UIMessage, std::__n1::basic_string_view<char, std::__n1::char_traits<char>>)
>>> defined at ohos_system_stubs.cpp:39
>>> defined at NativeApp.cpp:1504 in archive libUI.a

ld.lld: error: duplicate symbol: System_RunOnMainThread(std::__n1::function<void ()>)
>>> defined at ohos_system_stubs.cpp:104
>>> defined at NativeApp.cpp:1512 in archive libUI.a

ld.lld: error: duplicate symbol: NativeSaveSecret(...)
>>> defined at ohos_system_stubs.cpp:57
>>> defined at NativeApp.cpp:1590 in archive libUI.a

ld.lld: error: duplicate symbol: NativeLoadSecret(...)
>>> defined at ohos_system_stubs.cpp:49
>>> defined at NativeApp.cpp:1602 in archive libUI.a

ld.lld: error: duplicate symbol: System_AudioGetDebugStats(char*, unsigned long)
>>> defined at ohos_system_stubs.cpp:74
>>> defined at AudioCommon.cpp:47 in archive libUI.a

ld.lld: error: duplicate symbol: System_AudioClear()
>>> defined at ohos_system_stubs.cpp:72
>>> defined at AudioCommon.cpp:59 in archive libUI.a

ld.lld: error: duplicate symbol: System_AudioPushSamples(int const*, int, float)
>>> defined at ohos_system_stubs.cpp:81
>>> defined at AudioCommon.cpp:63 in archive libUI.a
```

## 原因分析

在集成 UI 库之前，我们在 `ohos_system_stubs.cpp` 中创建了这些函数的桩实现。但是 UI 库中的以下文件已经提供了真实实现：

### UI/NativeApp.cpp 中的函数
- `System_PostUIMessage()` - 发送 UI 消息
- `System_RunOnMainThread()` - 在主线程执行函数
- `NativeSaveSecret()` - 保存密钥
- `NativeLoadSecret()` - 加载密钥

### UI/AudioCommon.cpp 中的函数
- `System_AudioGetDebugStats()` - 获取音频调试信息
- `System_AudioClear()` - 清除音频缓冲
- `System_AudioPushSamples()` - 推送音频样本

## 解决方案

### 修改 ohos_system_stubs.cpp

从 `ohos_system_stubs.cpp` 中移除所有与 UI 库冲突的函数实现。

**修改前**:
```cpp
void System_PostUIMessage(UIMessage message, std::string_view value) {
    // 桩实现
}

void System_RunOnMainThread(std::function<void()> func) {
    // 桩实现
}

std::string NativeLoadSecret(std::string_view name) {
    // 桩实现
}

bool NativeSaveSecret(std::string_view name, std::string_view value) {
    // 桩实现
}

void System_AudioClear() {
    // 桩实现
}

void System_AudioGetDebugStats(char *buf, size_t bufSize) {
    // 桩实现
}

void System_AudioPushSamples(const int *audio, int numSamples, float rate) {
    // 桩实现
}
```

**修改后**:
```cpp
// 这些函数已在 UI 库中实现，不需要桩实现：
// - System_PostUIMessage (UI/NativeApp.cpp)
// - System_RunOnMainThread (UI/NativeApp.cpp)
// - NativeSaveSecret (UI/NativeApp.cpp)
// - NativeLoadSecret (UI/NativeApp.cpp)
// - System_AudioGetDebugStats (UI/AudioCommon.cpp)
// - System_AudioClear (UI/AudioCommon.cpp)
// - System_AudioPushSamples (UI/AudioCommon.cpp)

// 只保留 UI 库中没有实现的函数：
bool IsGameVRScene() {
    return false;
}

int64_t System_GetPropertyInt(SystemProperty prop) {
    // ...
}

void System_Notify(SystemNotification notification) {
    // ...
}

uint64_t CityHash64(const char *buf, size_t len) {
    // ...
}
```

## 保留的桩函数

以下函数仍然保留在 `ohos_system_stubs.cpp` 中，因为 UI 库没有提供实现：

1. **IsGameVRScene()** - VR 场景检测
2. **System_GetPropertyInt()** - 获取整数属性
3. **System_Notify()** - 系统通知
4. **CityHash64()** - 哈希函数

## 验证

### 编译验证

```bash
cd ohos
./gradlew clean
./gradlew assembleDebug
```

### 检查符号

```bash
# 检查 UI 库中的符号
nm libUI.a | grep -E "System_PostUIMessage|NativeSaveSecret|System_Audio"

# 检查最终库中的符号（应该只有一份）
nm -D libppsspp_ohos.so | grep -E "System_PostUIMessage|NativeSaveSecret|System_Audio"
```

## 函数实现位置总结

### UI/NativeApp.cpp
| 函数 | 行号 | 说明 |
|------|------|------|
| System_PostUIMessage | 1504 | 发送 UI 消息到主线程 |
| System_RunOnMainThread | 1512 | 在主线程执行函数 |
| NativeSaveSecret | 1590 | 保存加密密钥 |
| NativeLoadSecret | 1602 | 加载加密密钥 |

### UI/AudioCommon.cpp
| 函数 | 行号 | 说明 |
|------|------|------|
| System_AudioGetDebugStats | 47 | 获取音频调试统计 |
| System_AudioClear | 59 | 清除音频缓冲区 |
| System_AudioPushSamples | 63 | 推送音频样本 |

### ohos_system_stubs.cpp (保留)
| 函数 | 说明 |
|------|------|
| IsGameVRScene | VR 场景检测（返回 false） |
| System_GetPropertyInt | 获取整数属性 |
| System_Notify | 系统通知（空实现） |
| CityHash64 | 简单哈希实现 |

## 注意事项

### 1. 不要重复实现

在添加新的系统函数时，先检查 UI 库是否已经实现：

```bash
# 搜索函数定义
grep -r "function_name" UI/
```

### 2. 桩实现的作用

桩实现只在以下情况使用：
- UI 库未集成时的临时实现
- UI 库中确实没有实现的函数
- 平台特定的函数

### 3. 链接顺序

使用 `whole-archive` 确保所有符号都被链接：

```cmake
target_link_libraries(ppsspp_ohos
    PRIVATE
    -Wl,--whole-archive
    Common
    Core
    UI      # UI 库必须在这里
    -Wl,--no-whole-archive
)
```

## 相关文档

- [UI_INTEGRATION.md](UI_INTEGRATION.md) - UI 库集成说明
- [LINKING_FIXES.md](LINKING_FIXES.md) - 其他链接问题修复
- [COMPILATION_FIXES.md](COMPILATION_FIXES.md) - 编译问题修复

## 总结

通过移除 `ohos_system_stubs.cpp` 中与 UI 库冲突的函数实现，解决了重复符号错误。现在：

- ✅ UI 库提供真实的函数实现
- ✅ 桩文件只保留必要的平台特定函数
- ✅ 链接成功，无重复符号错误

下一步可以继续编译和测试。
