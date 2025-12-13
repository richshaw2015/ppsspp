# OHOS 文本渲染实现说明

## 当前状态

### 使用方案：字体图集（Font Atlas）

由于 HarmonyOS NAPI 的线程安全限制，当前 PPSSPP OHOS 版本使用**字体图集**进行文本渲染，而不是 TextDrawer。

### 为什么不使用 TextDrawer？

**问题**：HarmonyOS 的 ArkTS VM (`ecma_vm`) 不支持多线程调用 NAPI：
```
Fatal: ecma_vm cannot run in multi-thread! thread:6771 currentThread:6984
```

**原因**：
1. TextRenderer 在主线程（UI 线程）初始化
2. 文本渲染在渲染线程执行
3. 渲染线程调用 NAPI 函数（AllocFont, MeasureText, RenderText）
4. ArkTS VM 检测到跨线程调用，触发崩溃

### 字体图集方案

**优点**：
- ✅ 稳定可靠，无线程问题
- ✅ 性能好，纹理直接渲染
- ✅ 支持中文，`font_atlas.zim` 包含完整字符集

**实现**：
- 文件：`UI/UIAtlas.cpp`
- OHOS 加载：`font_atlas.zim`（包含中文字符）
- 其他平台：`asciifont_atlas.zim`（仅 ASCII）

## 未来改进方向

如果要启用 TextDrawer（ArkTS 动态文本渲染），需要实现以下方案之一：

### 方案 1：线程安全函数（推荐）

使用 `napi_threadsafe_function` API：

```cpp
// 创建线程安全函数
napi_threadsafe_function tsfn;
napi_create_threadsafe_function(
    env, 
    js_function,
    nullptr,
    work_name,
    0,
    1,
    nullptr,
    nullptr,
    nullptr,
    call_js_cb,
    &tsfn
);

// 在任意线程调用
napi_call_threadsafe_function(tsfn, data, napi_tsfn_blocking);
```

**优点**：
- NAPI 官方支持
- 线程安全
- 异步执行

**缺点**：
- 实现复杂
- 需要回调机制
- 可能有延迟

### 方案 2：消息队列

实现一个消息队列，将 NAPI 调用转发到主线程：

```cpp
struct NAPICall {
    enum Type { ALLOC_FONT, MEASURE_TEXT, RENDER_TEXT };
    Type type;
    std::string data;
    std::promise<Result> promise;
};

std::queue<NAPICall> g_napi_queue;
std::mutex g_queue_mutex;

// 渲染线程：
auto future = EnqueueNAPICall(call);
auto result = future.get();  // 等待主线程执行

// 主线程：
ProcessNAPIQueue();  // 在每帧执行
```

**优点**：
- 完全控制执行时机
- 可以批处理

**缺点**：
- 需要主线程轮询
- 实现复杂
- 可能阻塞渲染线程

### 方案 3：预渲染缓存

在主线程预先渲染常用文本：

```cpp
// 初始化时
PreRenderCommonTexts({
    "Settings", "Graphics", "Audio", 
    "设置", "图形", "音频",
    // ...
});

// 渲染时
auto texture = GetCachedText(text);
if (!texture) {
    // 回退到字体图集
}
```

**优点**：
- 简单
- 无线程问题
- 常用文本性能好

**缺点**：
- 无法处理动态文本
- 内存占用大

## 代码位置

### 字体图集
- `UI/UIAtlas.cpp` - 字体图集加载
- `Common/Render/DrawBuffer.cpp` - 字体图集渲染

### TextDrawer（当前禁用）
- `Common/Render/Text/draw_text_ohos.cpp` - OHOS TextDrawer 实现
- `ohos/entry/src/main/cpp/napi/napi_text_renderer.cpp` - NAPI 绑定
- `ohos/entry/src/main/ets/utils/TextRenderer.ets` - ArkTS 文本渲染

### 关键函数
- `OhosTextRenderer::IsReady()` - 当前返回 `false`
- `AtlasProvider()` - 加载字体图集

## 测试

### 验证中文显示
1. 启动应用
2. 检查主界面文字
3. 进入设置页面
4. 验证所有中文文本正常显示

### 日志
```
[System] Loading full font atlas for OHOS (with Chinese support)
[System] Successfully loaded font_atlas.zim for OHOS
[PPSSPP_TextRenderer] IsReady() returning false - using font atlas for stability
[G3D] UIContext::DrawText: Using font atlas
```

## 参考

- Android 实现：`Common/Render/Text/draw_text_android.cpp`
- NAPI 文档：https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/napi-guidelines-V5
- 线程安全函数：https://nodejs.org/api/n-api.html#n_api_threadsafe_function_example
