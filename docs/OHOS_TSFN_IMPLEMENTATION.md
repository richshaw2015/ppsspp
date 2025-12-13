# OHOS TextDrawer 线程安全函数（TSFN）实现

## 概述

使用 `napi_threadsafe_function` 实现跨线程文本渲染，解决 HarmonyOS NAPI 的线程限制问题。

## 架构

```
渲染线程                    主线程（JS 线程）
    |                            |
    | AllocFont()                |
    |------------------------->  |
    | napi_call_threadsafe_      |
    | function()                 |
    |                            |
    |                    TSFNCallJS()
    |                    调用 ArkTS
    |                    TextRenderer
    |                            |
    | <-------------------------|
    | 返回结果                   |
    |                            |
```

## 核心组件

### 1. 数据结构

```cpp
struct TSFNCallData {
    enum Type { ALLOC_FONT, MEASURE_TEXT, RENDER_TEXT };
    Type type;
    
    // 输入参数
    std::string text;
    std::string fontFile;
    int fontId;
    float size;
    
    // 输出结果
    int intResult;
    uint32_t uint32Result;
    std::vector<uint32_t> vectorResult;
    
    // 同步机制
    std::mutex mutex;
    std::condition_variable cv;
    bool completed;
};
```

### 2. 线程安全函数

```cpp
static napi_threadsafe_function g_tsfn_allocFont = nullptr;
static napi_threadsafe_function g_tsfn_measureText = nullptr;
static napi_threadsafe_function g_tsfn_renderText = nullptr;
```

### 3. 回调函数

```cpp
static void TSFNCallJS(napi_env env, napi_value js_callback, 
                       void* context, void* data) {
    TSFNCallData* callData = static_cast<TSFNCallData*>(data);
    
    // 在主线程执行 NAPI 调用
    // 根据 callData->type 调用不同的 ArkTS 方法
    // 将结果写入 callData
    // 通知等待的线程
    
    callData->completed = true;
    callData->cv.notify_one();
}
```

## 工作流程

### 初始化

```cpp
bool Initialize(napi_env env, napi_value textRendererClass) {
    // 1. 保存 env 和类引用
    g_main_env = env;
    napi_create_reference(env, textRendererClass, 1, &g_textRendererClass);
    
    // 2. 创建线程安全函数
    napi_create_threadsafe_function(
        env,
        nullptr,  // 不需要 JS 回调
        nullptr,  // async_resource
        async_resource_name,
        0,  // max_queue_size (unlimited)
        1,  // initial_thread_count
        nullptr,  // thread_finalize_data
        nullptr,  // thread_finalize_cb
        nullptr,  // context
        TSFNCallJS,  // call_js_cb
        &g_tsfn_allocFont
    );
    
    return true;
}
```

### 跨线程调用

```cpp
int AllocFont(const std::string& fontFile) {
    // 1. 创建调用数据
    TSFNCallData* callData = new TSFNCallData();
    callData->type = TSFNCallData::ALLOC_FONT;
    callData->fontFile = fontFile;
    
    // 2. 调用线程安全函数（非阻塞，立即返回）
    napi_call_threadsafe_function(
        g_tsfn_allocFont,
        callData,
        napi_tsfn_blocking  // 阻塞模式
    );
    
    // 3. 等待主线程执行完成
    std::unique_lock<std::mutex> lock(callData->mutex);
    callData->cv.wait(lock, [callData] { 
        return callData->completed; 
    });
    
    // 4. 获取结果
    int result = callData->intResult;
    delete callData;
    
    return result;
}
```

### 主线程执行

```cpp
static void TSFNCallJS(napi_env env, napi_value js_callback, 
                       void* context, void* data) {
    TSFNCallData* callData = static_cast<TSFNCallData*>(data);
    
    // 获取 TextRenderer 类
    napi_value classValue;
    napi_get_reference_value(env, g_textRendererClass, &classValue);
    
    if (callData->type == TSFNCallData::ALLOC_FONT) {
        // 调用 allocFont
        napi_value allocFontFunc;
        napi_get_named_property(env, classValue, "allocFont", &allocFontFunc);
        
        napi_value args[1];
        napi_create_string_utf8(env, callData->fontFile.c_str(), 
                                NAPI_AUTO_LENGTH, &args[0]);
        
        napi_value result;
        napi_call_function(env, classValue, allocFontFunc, 1, args, &result);
        
        napi_get_value_int32(env, result, &callData->intResult);
    }
    
    // 通知等待的线程
    callData->completed = true;
    callData->cv.notify_one();
}
```

## 优点

1. ✅ **线程安全**：完全解决跨线程调用问题
2. ✅ **官方支持**：使用 NAPI 标准 API
3. ✅ **同步语义**：调用者可以等待结果
4. ✅ **性能良好**：主线程异步执行，不阻塞
5. ✅ **稳定可靠**：不会崩溃

## 缺点

1. ❌ **实现复杂**：需要数据结构、同步机制
2. ❌ **内存管理**：需要手动管理 callData
3. ❌ **延迟**：需要等待主线程调度
4. ❌ **代码量大**：约 300+ 行代码

## 性能考虑

### 延迟分析

```
渲染线程调用 -> 队列 -> 主线程调度 -> 执行 -> 通知 -> 返回
              ~1ms     ~1-5ms       ~1ms   ~1ms   total: 4-8ms
```

### 优化建议

1. **批处理**：一次调用渲染多个文本
2. **缓存**：缓存常用文本的渲染结果
3. **预渲染**：在空闲时预渲染常用文本
4. **异步**：对于不需要立即结果的调用，使用异步模式

## 使用示例

### C++ 端

```cpp
// 初始化（在主线程）
OhosTextRenderer::Initialize(env, textRendererClass);

// 渲染线程调用
int fontId = OhosTextRenderer::AllocFont("Roboto-Regular.ttf");
uint32_t size = OhosTextRenderer::MeasureText("Hello", fontId, 24.0f);
std::vector<uint32_t> pixels = OhosTextRenderer::RenderText("Hello", fontId, 24.0f);
```

### ArkTS 端

```typescript
export class TextRenderer {
  static allocFont(fontFile: string): number {
    // 加载字体，返回 fontId
    return fontId;
  }
  
  static measureText(text: string, fontId: number, size: number): number {
    // 测量文本，返回打包的宽高
    return (width << 16) | height;
  }
  
  static renderText(text: string, fontId: number, size: number): Uint32Array {
    // 渲染文本，返回像素数据
    return pixels;
  }
}
```

## 测试

### 验证线程安全

```cpp
// 在多个线程同时调用
std::thread t1([] {
    OhosTextRenderer::RenderText("Thread 1", fontId, 24.0f);
});

std::thread t2([] {
    OhosTextRenderer::RenderText("Thread 2", fontId, 24.0f);
});

t1.join();
t2.join();
// 应该不会崩溃
```

### 性能测试

```cpp
auto start = std::chrono::high_resolution_clock::now();

for (int i = 0; i < 100; i++) {
    OhosTextRenderer::RenderText("Test", fontId, 24.0f);
}

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

// 预期：400-800ms（100次调用，每次4-8ms）
```

## 故障排除

### 问题：调用超时

**原因**：主线程繁忙，无法及时处理 TSFN 调用

**解决**：
1. 增加 max_queue_size
2. 使用非阻塞模式
3. 优化主线程性能

### 问题：内存泄漏

**原因**：callData 未正确释放

**解决**：
1. 确保每个 new 都有对应的 delete
2. 使用智能指针（std::unique_ptr）
3. 添加异常处理

### 问题：数据竞争

**原因**：多个线程同时访问 callData

**解决**：
1. 使用 mutex 保护
2. 确保 completed 标志正确设置
3. 使用 condition_variable 同步

## 参考

- NAPI 文档：https://nodejs.org/api/n-api.html#n_api_threadsafe_function_example
- HarmonyOS NAPI：https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/napi-guidelines-V5
- 实现代码：`ohos/entry/src/main/cpp/napi/napi_text_renderer.cpp`
