# OHOS TextDrawer 实现文档

## 概述

参考 Android 的 `TextDrawerAndroid` 实现，为 OHOS 平台创建 `TextDrawerOHOS`，使用 HarmonyOS 的 Canvas API 动态渲染文本。

## 实现架构

```
ArkTS 层 (TextRenderer.ets)
    ↓ 使用 HarmonyOS Canvas API
    ↓ drawing.Canvas, drawing.Paragraph
    ↓
NAPI 层 (napi_text_renderer.cpp)
    ↓ JNI 风格的绑定
    ↓ OhosTextRenderer 命名空间
    ↓
C++ 层 (draw_text_ohos.cpp)
    ↓ TextDrawerOHOS 类
    ↓ 实现 TextDrawer 接口
    ↓
PPSSPP Core
    ↓ 使用 TextDrawer::DrawString()
    ↓ 显示 UI 文本
```

## 已创建的文件

### 1. C++ 头文件
**文件**: `Common/Render/Text/draw_text_ohos.h`

```cpp
class TextDrawerOHOS : public TextDrawer {
public:
    TextDrawerOHOS(Draw::DrawContext *draw);
    ~TextDrawerOHOS();
    
    bool IsReady() const override;
    void SetOrCreateFont(const FontStyle &style) override;
    bool DrawStringBitmap(...) override;
    
protected:
    void MeasureStringInternal(...) override;
    bool SupportsColorEmoji() const override { return true; }
    void ClearFonts() override;
    
private:
    std::map<FontStyle, OhosFontEntry> fontMap_;
    std::map<std::string, int> allocatedFonts_;
};
```

### 2. C++ 实现文件
**文件**: `Common/Render/Text/draw_text_ohos.cpp`

**关键功能**:
- `SetOrCreateFont()` - 分配字体
- `MeasureStringInternal()` - 测量文本大小
- `DrawStringBitmap()` - 渲染文本到位图
- 像素格式转换（ARGB → 纹理格式）

### 3. NAPI 绑定
**文件**: `ohos/entry/src/main/cpp/napi/napi_text_renderer.cpp`

**命名空间**: `OhosTextRenderer`

**函数**:
- `Initialize()` - 初始化文本渲染器
- `IsReady()` - 检查是否就绪
- `AllocFont()` - 分配字体
- `FreeAllFonts()` - 释放所有字体
- `MeasureText()` - 测量文本
- `RenderText()` - 渲染文本

### 4. ArkTS 文本渲染器
**文件**: `ohos/entry/src/main/ets/utils/TextRenderer.ets`

**类**: `TextRenderer`

**方法**:
- `allocFont(fontFile: string): number` - 分配字体
- `freeAllFonts(): void` - 释放字体
- `measureText(text: string, fontId: number, textSize: number): number` - 测量文本
- `renderText(text: string, fontId: number, textSize: number): Uint32Array` - 渲染文本

**使用的 HarmonyOS API**:
- `drawing.Canvas` - 画布
- `drawing.TextStyle` - 文本样式
- `drawing.ParagraphStyle` - 段落样式
- `drawing.ParagraphBuilder` - 段落构建器
- `drawing.Paragraph` - 段落
- `image.PixelMap` - 像素图

### 5. 集成修改

#### `Common/Render/Text/draw_text.cpp`
```cpp
#include "Common/Render/Text/draw_text_ohos.h"

TextDrawer *TextDrawer::Create(Draw::DrawContext *draw) {
    // ...
    #elif PPSSPP_PLATFORM(OHOS)
        drawer = new TextDrawerOHOS(draw);
    // ...
}
```

#### `ohos/entry/src/main/cpp/napi/napi_init.cpp`
```cpp
static napi_value InitTextRenderer(napi_env env, napi_callback_info info) {
    // 注册 TextRenderer 类
}

// 在模块导出中添加
{"initTextRenderer", nullptr, InitTextRenderer, ...},
```

#### `ohos/entry/src/main/ets/pages/Index.ets`
```typescript
import { TextRenderer } from '../utils/TextRenderer';

async aboutToAppear(): Promise<void> {
    // 初始化文本渲染器
    ppsspp.initTextRenderer(TextRenderer);
    
    // 初始化 PPSSPP 核心
    ppsspp.initEmulator(...);
}
```

## 工作流程

### 1. 初始化阶段

```typescript
// ArkTS 层
ppsspp.initTextRenderer(TextRenderer);
    ↓
// NAPI 层
InitTextRenderer(env, TextRenderer类)
    ↓
// 保存类引用供后续使用
```

### 2. 字体分配

```cpp
// C++ 层
TextDrawerOHOS::SetOrCreateFont(style)
    ↓
OhosTextRenderer::AllocFont("Roboto-Regular.ttf")
    ↓
// NAPI 调用 ArkTS
TextRenderer.allocFont("Roboto-Regular.ttf")
    ↓
// 返回字体 ID
```

### 3. 文本测量

```cpp
// C++ 层
TextDrawerOHOS::MeasureStringInternal("设置")
    ↓
OhosTextRenderer::MeasureText("设置", fontId, size)
    ↓
// NAPI 调用 ArkTS
TextRenderer.measureText("设置", fontId, size)
    ↓
// HarmonyOS API
drawing.Paragraph.layout()
drawing.Paragraph.getMaxIntrinsicWidth()
drawing.Paragraph.getHeight()
    ↓
// 返回打包的宽高
return (width << 16) | height;
```

### 4. 文本渲染

```cpp
// C++ 层
TextDrawerOHOS::DrawStringBitmap("设置")
    ↓
OhosTextRenderer::RenderText("设置", fontId, size)
    ↓
// NAPI 调用 ArkTS
TextRenderer.renderText("设置", fontId, size)
    ↓
// HarmonyOS API
image.createPixelMapSync()  // 创建位图
drawing.Canvas(pixelMap)    // 创建画布
drawing.ParagraphBuilder    // 构建段落
paragraph.paint(canvas)     // 绘制文本
pixelMap.readPixelsSync()   // 读取像素
    ↓
// 返回 ARGB 像素数组
return Uint32Array
    ↓
// C++ 层转换像素格式
转换为 OpenGL 纹理格式
    ↓
// 上传到 GPU
创建纹理并显示
```

## 像素格式转换

### RGBA → ARGB
```typescript
// ArkTS 层
const rgba = pixels[i];
const r = (rgba >> 0) & 0xFF;
const g = (rgba >> 8) & 0xFF;
const b = (rgba >> 16) & 0xFF;
const a = (rgba >> 24) & 0xFF;
result[i] = (a << 24) | (r << 16) | (g << 8) | b;
```

### ARGB → 纹理格式
```cpp
// C++ 层
if (texFormat == Draw::DataFormat::R8G8B8A8_UNORM) {
    uint32_t v = imageData[i];
    // Swap R and B
    v = (v & 0xFF00FF00) | ((v >> 16) & 0xFF) | ((v << 16) & 0xFF0000);
    bitmapData32[i] = RGBAToPremul8888(v);
}
```

## 当前状态

### ✅ 已完成
1. ✅ C++ 头文件和实现文件
2. ✅ NAPI 绑定框架
3. ✅ ArkTS 文本渲染器
4. ✅ 集成到 PPSSPP 核心
5. ✅ 初始化流程

### ⚠️ 待完善
1. ⚠️ NAPI 层的完整实现（目前是桩）
2. ⚠️ 字体文件加载（从 rawfile）
3. ⚠️ 错误处理和日志
4. ⚠️ 性能优化
5. ⚠️ 内存管理

### 🔧 需要实现的功能

#### 1. 完整的 NAPI 绑定
```cpp
// napi_text_renderer.cpp
int AllocFont(const std::string& fontFile) {
    // 调用 ArkTS 层的 TextRenderer.allocFont()
    // 需要保存 napi_env 和方法引用
}

uint32_t MeasureText(const std::string& text, int fontId, float size) {
    // 调用 ArkTS 层的 TextRenderer.measureText()
}

std::vector<uint32_t> RenderText(const std::string& text, int fontId, float size) {
    // 调用 ArkTS 层的 TextRenderer.renderText()
    // 需要转换 Uint32Array 到 std::vector
}
```

#### 2. 字体文件加载
```typescript
// TextRenderer.ets
static allocFont(fontFile: string): number {
    // 从 rawfile 加载字体
    const fontData = context.resourceManager.getRawFileContent(fontFile);
    
    // 创建字体集合
    const fontCollection = new drawing.FontCollection();
    // TODO: 加载字体数据到 fontCollection
    
    return fontId;
}
```

#### 3. 线程安全
- NAPI 调用需要在正确的线程上执行
- 可能需要使用 `napi_threadsafe_function`

## 测试建议

### 1. 基础测试
```cpp
// 测试文本测量
float w, h;
drawer->MeasureString("Hello", &w, &h);
// 验证 w > 0 && h > 0
```

### 2. 中文测试
```cpp
// 测试中文渲染
drawer->DrawString(target, "设置", x, y, color);
// 验证文本正确显示
```

### 3. Emoji 测试
```cpp
// 测试 Emoji
drawer->DrawString(target, "😀🎮", x, y, color);
// 验证 Emoji 显示
```

### 4. 性能测试
```cpp
// 测试缓存
for (int i = 0; i < 1000; i++) {
    drawer->DrawString(target, "相同文本", x, y, color);
}
// 验证第二次及以后使用缓存
```

## 与 Android 的对比

| 特性 | Android | OHOS |
|------|---------|------|
| 语言 | Java | ArkTS |
| 画布 API | android.graphics.Canvas | drawing.Canvas |
| 字体 API | android.graphics.Typeface | drawing.FontCollection |
| 位图 API | android.graphics.Bitmap | image.PixelMap |
| 绑定方式 | JNI | NAPI |
| 线程模型 | JNI AttachCurrentThread | NAPI threadsafe function |

## 参考文档

- [HarmonyOS Canvas API](https://developer.huawei.com/consumer/cn/doc/harmonyos-references-V5/js-apis-graphics-drawing-V5)
- [HarmonyOS Image API](https://developer.huawei.com/consumer/cn/doc/harmonyos-references-V5/js-apis-image-V5)
- [PPSSPP TextDrawer](https://github.com/hrydgard/ppsspp/tree/master/Common/Render/Text)
- [Android TextRenderer](https://github.com/hrydgard/ppsspp/blob/master/android/src/org/ppsspp/ppsspp/TextRenderer.java)

## 下一步

1. **完善 NAPI 绑定** - 实现真正的 ArkTS 调用
2. **加载字体文件** - 从 rawfile 读取 TTF 文件
3. **测试和调试** - 验证文本正确渲染
4. **性能优化** - 确保文本缓存工作正常
5. **错误处理** - 添加完善的错误处理逻辑

## 总结

TextDrawerOHOS 的实现框架已经完成，参考了 Android 的成熟实现。主要工作集中在 NAPI 绑定和 ArkTS 层的文本渲染。一旦完善，PPSSPP 将能够在 OHOS 平台上显示所有 Unicode 字符，包括中文、Emoji 等。
