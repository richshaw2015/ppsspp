# OHOS 音频实现 - 编译修复

## 遇到的问题

### 1. 找不到头文件
```
fatal error: 'native_audiostreambuilder.h' file not found
```

**原因**：头文件路径不正确

**解决方案**：使用正确的路径
```cpp
// 错误
#include <native_audiostreambuilder.h>
#include <native_audiorenderer.h>

// 正确
#include <ohaudio/native_audiostreambuilder.h>
#include <ohaudio/native_audiorenderer.h>
```

### 2. ohos_app.cpp 使用了旧的 API
```
error: use of undeclared identifier 'OhosAudio'
```

**原因**：`ohos_app.cpp` 使用了旧的命名空间 API，但音频现在通过 `AudioBackend` 接口管理

**解决方案**：移除直接的音频调用
```cpp
// 移除这些调用
OhosAudio::Initialize(44100, 512);
OhosAudio::Shutdown();
OhosAudio::Pause();
OhosAudio::Resume();

// 音频由 PPSSPP Core 的 AudioBackend 管理
// 在 NativeInit() 中自动初始化
// 在 NativeShutdown() 中自动清理
```

### 3. 链接错误
```
ld.lld: error: unable to find library -lohaudio.z
```

**原因**：库名称不正确

**解决方案**：使用正确的库名
```cmake
# 错误
target_link_libraries(ppsspp_ohos PUBLIC ohaudio.z)

# 正确
target_link_libraries(ppsspp_ohos PUBLIC ohaudio)
```

### 4. 回调函数 API
```
OH_AudioStreamBuilder_SetRendererWriteDataCallback
```

**原因**：SDK 中只有 Advanced 版本的回调

**解决方案**：使用 Advanced 回调
```cpp
// 使用这个
OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced(builder_, callback, userData);

// 回调签名
typedef int32_t (*OH_AudioRenderer_OnWriteDataCallbackAdvanced)(
    OH_AudioRenderer* renderer,
    void* userData,
    void* audioData,
    int32_t audioDataSize);

// 返回值：写入的字节数
return audioDataSize;
```

## 修复的文件

1. **ohos/entry/src/main/cpp/ohos_audio.cpp**
   - 修正头文件路径
   - 使用 Advanced 回调 API
   - 修正回调返回值

2. **ohos/entry/src/main/cpp/ohos_app.cpp**
   - 移除直接的音频 API 调用
   - 添加注释说明音频由 AudioBackend 管理

3. **ohos/entry/src/main/cpp/CMakeLists.txt**
   - 修正库名称：`ohaudio.z` → `ohaudio`

## 验证

### 检查头文件位置
```bash
find /Applications/DevEco-Studio.app/Contents/sdk/default -name "native_audio*.h"
# 应该在: .../sysroot/usr/include/ohaudio/
```

### 检查库文件
```bash
ls /Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/sysroot/usr/lib/aarch64-linux-ohos/ | grep audio
# 应该看到: libohaudio.so
```

### 编译测试
```bash
cd ohos
./rebuild.sh
# 应该成功编译
```

## SDK 版本要求

- **最低版本**：HarmonyOS NEXT SDK (API 12+)
- **推荐版本**：最新的 HarmonyOS NEXT SDK
- **OHAudio API**：从 API 10 开始可用

## 注意事项

1. **头文件路径**
   - 所有 OHAudio 头文件都在 `ohaudio/` 目录下
   - 不要使用 `multimedia/player_framework/` 路径

2. **库链接**
   - 使用 `ohaudio` 而不是 `ohaudio.z`
   - `.z` 后缀用于某些系统库，但不是所有库

3. **回调函数**
   - 使用 Advanced 版本的回调
   - 返回值必须是写入的字节数
   - 返回值范围：[0, audioDataSize]

4. **音频管理**
   - 不要在应用层直接管理音频
   - 让 PPSSPP Core 通过 AudioBackend 管理
   - 暂停/恢复由 Core 自动处理

## 相关文档

- `AUDIO_IMPLEMENTATION.md` - 完整实现文档
- `AUDIO_QUICK_START.md` - 快速开始指南
- [OHAudio API 文档](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/ohaudio-overview-V5)
