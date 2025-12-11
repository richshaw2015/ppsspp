# 音频无声问题修复

## 问题

设置里打开了按键音效，但是没有声音输出。

## 根本原因

**OHOS 代码从未调用 `NativeInit()`！**

PPSSPP 的音频系统在 `NativeInit()` 中初始化：

```cpp
// UI/NativeApp.cpp
void NativeInit(...) {
    // ...
    g_audioBackend = System_CreateAudioBackend();  // ← 创建音频后端
    if (g_audioBackend) {
        g_audioBackend->SetRenderCallback(&NativeMixWrapper, nullptr);
        // ...
    }
}
```

但是 OHOS 的 `OhosApp::Initialize()` 只初始化了系统接口和输入系统，**没有调用 `NativeInit()`**，导致：
- ❌ 音频后端未创建
- ❌ 音频回调未设置
- ❌ 没有声音输出

## 解决方案

在 `OhosApp::Initialize()` 中调用 `NativeInit()`：

```cpp
// ohos/entry/src/main/cpp/ohos_app.cpp

bool Initialize() {
    // ...
    
    // 初始化 PPSSPP Core
    OHOS_LOGI(APP_TAG, "Calling NativeInit...");
    
    std::string filesDir = OhosSystem::GetFilesDirectory();
    std::string cacheDir = OhosSystem::GetCacheDirectory();
    
    const char *argv[] = {"ppsspp"};
    NativeInit(1, argv, filesDir.c_str(), filesDir.c_str(), cacheDir.c_str());
    
    OHOS_LOGI(APP_TAG, "NativeInit completed");
    
    // ...
}
```

同样，在 `Shutdown()` 中调用 `NativeShutdown()`：

```cpp
void Shutdown() {
    // ...
    
    // 清理 PPSSPP Core（会清理音频系统）
    OHOS_LOGI(APP_TAG, "Calling NativeShutdown...");
    NativeShutdown();
    OHOS_LOGI(APP_TAG, "NativeShutdown completed");
    
    // ...
}
```

## 初始化流程

### 修复前（错误）

```
Index.ets: ppsspp.initEmulator()
    ↓
napi_ppsspp.cpp: InitEmulator()
    ↓
ohos_app.cpp: OhosApp::Initialize()
    ↓
初始化系统接口 ✅
初始化输入系统 ✅
❌ 没有调用 NativeInit()
❌ 音频系统未初始化
```

### 修复后（正确）

```
Index.ets: ppsspp.initEmulator()
    ↓
napi_ppsspp.cpp: InitEmulator()
    ↓
ohos_app.cpp: OhosApp::Initialize()
    ↓
初始化系统接口 ✅
初始化输入系统 ✅
调用 NativeInit() ✅
    ↓
UI/NativeApp.cpp: NativeInit()
    ↓
System_CreateAudioBackend() ✅
    ↓
new OhosAudioBackend() ✅
    ↓
InitOutputDevice() ✅
    ↓
OHAudio 初始化 ✅
    ↓
音频回调设置 ✅
    ↓
🎵 有声音了！
```

## NativeInit 的作用

`NativeInit()` 是 PPSSPP 的核心初始化函数，负责：

1. **CPU 特性检测**
2. **文件系统初始化**
3. **配置加载**
4. **音频系统初始化** ← 关键！
5. **图形系统准备**
6. **UI 系统初始化**

不调用 `NativeInit()` 就像盖房子没打地基，很多功能都无法正常工作。

## 验证

### 1. 查看日志

```bash
hdc shell hilog -x | grep -E "NativeInit|System_CreateAudioBackend|OhosAudioBackend"
```

应该看到：
```
[PPSSPP_App] Calling NativeInit...
[System] System_CreateAudioBackend: creating OhosAudioBackend
[Audio] OhosAudioBackend created
[Audio] Audio output device initialized: rate=44100, buffer=512
[PPSSPP_App] NativeInit completed
```

### 2. 测试音频

1. 启动 PPSSPP
2. 进入设置 → 音频
3. 打开"按键音效"
4. 点击任意按钮
5. **应该听到声音** ✅

### 3. 检查音频流

```bash
hdc shell hilog -x | grep "PPSSPP_Audio"
```

应该看到：
```
[PPSSPP_Audio] OhosAudioContext created: frames=512, rate=44100
[PPSSPP_Audio] Initializing OHAudio renderer...
[PPSSPP_Audio] OHAudio renderer started
```

## 其他平台对比

### Android
```cpp
// android/jni/app-android.cpp
extern "C" void Java_org_ppsspp_ppsspp_NativeApp_init(...) {
    // ...
    NativeInit(...);  // ✅ 调用了
    // ...
}
```

### iOS
```objc
// ios/main.mm
- (void)applicationDidFinishLaunching {
    // ...
    NativeInit(...);  // ✅ 调用了
    // ...
}
```

### SDL (Desktop)
```cpp
// SDL/SDLMain.cpp
int main(int argc, char *argv[]) {
    // ...
    NativeInit(argc, argv, ...);  // ✅ 调用了
    // ...
}
```

### OHOS (修复前)
```cpp
// ohos/entry/src/main/cpp/ohos_app.cpp
bool Initialize() {
    // ...
    // ❌ 没有调用 NativeInit()
}
```

### OHOS (修复后)
```cpp
// ohos/entry/src/main/cpp/ohos_app.cpp
bool Initialize() {
    // ...
    NativeInit(...);  // ✅ 现在调用了
}
```

## 修改的文件

- `ohos/entry/src/main/cpp/ohos_app.cpp`
  - 添加 `#include "Common/System/NativeApp.h"`
  - 在 `Initialize()` 中调用 `NativeInit()`
  - 在 `Shutdown()` 中调用 `NativeShutdown()`

## 注意事项

1. **目录路径**
   - `NativeInit()` 需要三个目录参数
   - 使用 `OhosSystem::GetFilesDirectory()` 和 `GetCacheDirectory()`

2. **调用顺序**
   - 必须先初始化系统接口（`OhosSystem`）
   - 然后才能调用 `NativeInit()`

3. **线程安全**
   - `NativeInit()` 必须在主线程调用
   - 音频回调会在独立线程中执行

4. **错误处理**
   - `NativeInit()` 没有返回值
   - 通过日志检查是否成功

## 相关文档

- `AUDIO_IMPLEMENTATION.md` - 音频实现文档
- `AUDIO_BUILD_FIXES.md` - 编译修复文档
- `AUDIO_QUICK_START.md` - 快速开始指南
- `Common/System/NativeApp.h` - NativeInit 接口定义
- `UI/NativeApp.cpp` - NativeInit 实现

## 总结

这是一个**关键性的修复**！

没有调用 `NativeInit()` 就像：
- 🚗 买了车但没加油
- 💡 装了灯但没通电
- 📱 买了手机但没装 SIM 卡

现在修复后，PPSSPP 的所有核心功能都能正常工作了，包括音频系统！🎵
