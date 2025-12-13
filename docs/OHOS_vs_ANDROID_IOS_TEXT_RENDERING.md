# OHOS vs Android/iOS 文本渲染实现对比

## Android 实现

### 线程模型
**JNI 线程附加机制（Thread Attachment）**

```cpp
// 1. 注册附加/分离函数（在 JNI_OnLoad 时）
void Android_AttachThreadToJNI() {
    JNIEnv *env;
    int status = gJvm->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (status < 0) {
        // 线程未附加到 JVM，需要附加
        JavaVMAttachArgs args{};
        args.version = JNI_VERSION_1_6;
        args.name = GetCurrentThreadName();
        status = gJvm->AttachCurrentThread(&env, &args);
    }
}

RegisterAttachDetach(&Android_AttachThreadToJNI, &Android_DetachThreadFromJNI);

// 2. 在任何需要调用 JNI 的线程中
AttachThreadToJNI();  // 附加当前线程到 JVM

// 3. 然后可以安全调用 JNI
auto env = getEnv();  // 获取当前线程的 JNIEnv
env->CallStaticIntMethod(cls, method, ...);
```

### 关键特性
- ✅ **支持多线程**：每个线程可以有自己的 `JNIEnv`
- ✅ **线程附加**：通过 `AttachCurrentThread` 附加线程到 JVM
- ✅ **线程安全**：附加后可以安全调用 JNI
- ✅ **自动管理**：通过 `RegisterAttachDetach` 自动处理

### 文本渲染流程
```
渲染线程 -> AttachThreadToJNI() 
         -> getEnv() 
         -> CallStaticIntMethod(TextRenderer.renderText) 
         -> 返回像素数据
```

### 代码位置
- `android/jni/app-android.cpp` - JNI 线程管理
- `Common/Render/Text/draw_text_android.cpp` - TextDrawer 实现
- `android/src/org/ppsspp/ppsspp/TextRenderer.java` - Java 文本渲染

---

## iOS 实现

### 线程模型
**原生 API（无跨语言调用）**

```objc
// 直接使用 CoreText/CoreGraphics API
CTFontRef font = CTFontCreateWithName(fontName, fontSize, nil);
CGContextRef context = CGBitmapContextCreate(...);

// 在任意线程直接调用，无需附加
CTLineDraw(line, context);
```

### 关键特性
- ✅ **完全线程安全**：CoreText API 本身就是线程安全的
- ✅ **无虚拟机**：不涉及运行时环境或虚拟机
- ✅ **原生性能**：C/Objective-C 直接调用，无开销
- ✅ **简单直接**：无需线程附加或特殊处理

### 文本渲染流程
```
渲染线程 -> CTFontCreateWithName() 
         -> CGBitmapContextCreate() 
         -> CTLineDraw() 
         -> 返回像素数据
```

### 代码位置
- `Common/Render/Text/draw_text_cocoa.mm` - TextDrawer 实现
- 使用 CoreText/CoreGraphics 框架

---

## OHOS 当前实现

### 线程模型
**NAPI 单线程限制**

```cpp
// 问题：NAPI 不支持跨线程调用
napi_env env = ...;  // 在主线程初始化
napi_call_function(env, ...);  // 在渲染线程调用 -> 崩溃！

// 错误信息：
// Fatal: ecma_vm cannot run in multi-thread! thread:6771 currentThread:6984
```

### 当前方案
**使用字体图集（Font Atlas）**

```cpp
bool IsReady() {
    // 返回 false，强制使用字体图集
    return false;
}
```

### 关键限制
- ❌ **不支持多线程**：ArkTS VM 不允许跨线程调用
- ❌ **无线程附加 API**：NAPI 没有提供类似 `AttachCurrentThread` 的机制
- ❌ **单 env 限制**：一个 `napi_env` 只能在创建它的线程使用

### 文本渲染流程
```
渲染线程 -> DrawBuffer 
         -> 字体图集（font_atlas.zim）
         -> 纹理渲染
```

### 代码位置
- `Common/Render/Text/draw_text_ohos.cpp` - TextDrawer 实现（当前禁用）
- `ohos/entry/src/main/cpp/napi/napi_text_renderer.cpp` - NAPI 绑定
- `UI/UIAtlas.cpp` - 字体图集加载

---

## 对比总结

| 特性 | Android | iOS | OHOS (当前) |
|------|---------|-----|-------------|
| **线程模型** | JNI 线程附加 | 原生 API | NAPI 单线程 |
| **多线程支持** | ✅ 是 | ✅ 是 | ❌ 否 |
| **线程附加 API** | `AttachCurrentThread` | 不需要 | ❌ 无 |
| **跨线程调用** | ✅ 支持 | ✅ 支持 | ❌ 崩溃 |
| **实现方式** | JNI + Java | CoreText | 字体图集 |
| **中文支持** | ✅ 完整 | ✅ 完整 | ✅ 完整 |
| **动态渲染** | ✅ 是 | ✅ 是 | ❌ 否（使用预渲染图集）|

---

## OHOS 的可能解决方案

### 方案 1：等待 HarmonyOS 提供线程附加 API
类似 Android 的 `AttachCurrentThread`，让 NAPI 支持多线程。

**优点**：
- 最彻底的解决方案
- 与 Android 模型一致

**缺点**：
- 需要等待 HarmonyOS 系统更新
- 时间不确定

### 方案 2：使用 napi_threadsafe_function
NAPI 提供的线程安全函数机制。

**优点**：
- NAPI 官方支持
- 可以立即实现

**缺点**：
- 实现复杂（需要回调、队列等）
- 异步执行，可能有延迟
- 需要大量重构代码

### 方案 3：继续使用字体图集（推荐）
当前方案，使用预渲染的字体纹理。

**优点**：
- ✅ 稳定可靠，零崩溃
- ✅ 性能优秀
- ✅ 完整中文支持
- ✅ 代码简单
- ✅ 已经可用

**缺点**：
- 无法动态调整字体大小
- 无法使用特殊字体效果
- 纹理内存占用

---

## 结论

**Android 和 iOS 都有成熟的多线程文本渲染方案**：
- Android 通过 JNI 线程附加
- iOS 通过线程安全的原生 API

**OHOS 目前缺少类似机制**：
- NAPI 不支持跨线程调用
- 没有线程附加 API
- 需要等待系统更新或使用复杂的 workaround

**最佳实践**：
- 当前使用字体图集是最稳定的方案
- 等待 HarmonyOS 提供更好的多线程 NAPI 支持
- 或者实现 napi_threadsafe_function（工作量大）

---

## 参考资料

- Android JNI 文档：https://developer.android.com/training/articles/perf-jni
- iOS CoreText 文档：https://developer.apple.com/documentation/coretext
- HarmonyOS NAPI 文档：https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/napi-guidelines-V5
- PPSSPP Android 实现：`android/jni/app-android.cpp`
- PPSSPP iOS 实现：`Common/Render/Text/draw_text_cocoa.mm`
