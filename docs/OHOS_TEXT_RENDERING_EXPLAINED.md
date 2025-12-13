# PPSSPP 文本渲染机制详解

## 问题

为什么 Android 平台只加载 `asciifont_atlas.zim`（ASCII 字体），却能正常显示中文？

```cpp
#if PPSSPP_PLATFORM(WINDOWS) || PPSSPP_PLATFORM(ANDROID) || PPSSPP_PLATFORM(MAC) || PPSSPP_PLATFORM(IOS)
    // Load the smaller ascii font only, like on Android. For debug ui etc.
    LoadAtlasMetadata(font_atlas, "asciifont_atlas.meta");
    fontTexture = CreateTextureFromFile(draw, "asciifont_atlas.zim", ImageFileType::ZIM, false);
#else
    // Load the full font texture.
    LoadAtlasMetadata(font_atlas, "font_atlas.meta");
    fontTexture = CreateTextureFromFile(draw, "font_atlas.zim", ImageFileType::ZIM, false);
#endif
```

## 答案：双层文本渲染系统

PPSSPP 使用了**两套独立的文本渲染系统**：

### 1. 图集字体（Atlas Font）- 用于 UI 图标和符号

**文件**: `asciifont_atlas.zim` 或 `font_atlas.zim`

**用途**:
- ✅ UI 图标（按钮、箭头等）
- ✅ 特殊符号
- ✅ 简单的 ASCII 字符（可选）

**特点**:
- 预渲染的位图字体
- 快速但字符集有限
- 主要用于 UI 装饰元素

### 2. 动态文本渲染（TextDrawer）- 用于所有文本

**实现**: 平台原生文本渲染 API

**Android 实现**:
```
C++ (TextDrawerAndroid)
    ↓ JNI 调用
Java (TextRenderer)
    ↓ Android Canvas API
系统字体渲染
```

**用途**:
- ✅ 所有 UI 文本（菜单、设置、对话框等）
- ✅ 支持所有 Unicode 字符（中文、日文、韩文、阿拉伯文等）
- ✅ 支持 Emoji 表情
- ✅ 使用系统字体

## 详细工作流程

### Android 平台

#### 1. C++ 层 (TextDrawerAndroid)

```cpp
// Common/Render/Text/draw_text_android.cpp

TextDrawerAndroid::TextDrawerAndroid(Draw::DrawContext *draw) : TextDrawer(draw) {
    // 通过 JNI 获取 Java 类
    const char *textRendererClassName = "org/ppsspp/ppsspp/TextRenderer";
    cls_textRenderer = ...;
    
    // 获取 Java 方法
    method_measureText = env->GetStaticMethodID(cls_textRenderer, "measureText", ...);
    method_renderText = env->GetStaticMethodID(cls_textRenderer, "renderText", ...);
}

bool TextDrawerAndroid::DrawStringBitmap(..., std::string_view str, ...) {
    // 1. 将 UTF-8 字符串转换为 Java 字符串
    jstring jstr = env->NewStringUTF(text.c_str());
    
    // 2. 调用 Java 方法渲染文本
    jintArray imageData = (jintArray)env->CallStaticObjectMethod(
        cls_textRenderer, method_renderText, jstr, font, size
    );
    
    // 3. 获取渲染后的位图数据
    jint *jimage = env->GetIntArrayElements(imageData, nullptr);
    
    // 4. 转换为纹理格式
    // ... 复制像素数据到 bitmapData ...
    
    return true;
}
```

#### 2. Java 层 (TextRenderer)

```java
// android/src/org/ppsspp/ppsspp/TextRenderer.java

public static int[] renderText(String string, int font, double textSize) {
    // 1. 设置字体和大小
    textPaint.setTypeface(fontMap.get(font));
    textPaint.setTextSize((float) textSize);
    
    // 2. 测量文本大小
    Point s = measure(string, font, textSize);
    
    // 3. 创建位图
    Bitmap bmp = Bitmap.createBitmap(s.x, s.y, Bitmap.Config.ARGB_8888);
    Canvas canvas = new Canvas(bmp);
    
    // 4. 使用 Android Canvas API 渲染文本
    // 这里会使用系统字体，支持所有 Unicode 字符！
    canvas.drawText(line, 1, -textPaint.ascent() + y, textPaint);
    
    // 5. 返回像素数据
    int[] pixels = new int[w * h];
    bmp.getPixels(pixels, 0, w, 0, 0, w, h);
    return pixels;
}
```

### 其他平台

- **Windows**: 使用 DirectWrite API
- **macOS/iOS**: 使用 CoreText API
- **Linux**: 使用 FreeType 或 Pango
- **OHOS**: 需要实现类似的原生文本渲染

## 为什么这样设计？

### 优势

1. **支持所有语言**
   - 使用系统字体，自动支持所有 Unicode 字符
   - 不需要打包大量字体文件

2. **节省空间**
   - `asciifont_atlas.zim`: ~100KB（只包含 ASCII 和图标）
   - `font_atlas.zim`: ~2MB（包含常用中文字符）
   - 系统字体: 0 字节（系统提供）

3. **更好的显示效果**
   - 使用系统字体渲染引擎
   - 支持字体平滑、提示等高级特性
   - 支持 Emoji 彩色表情

4. **灵活性**
   - 用户可以使用系统字体设置
   - 支持粗体、斜体等样式
   - 支持动态字体大小

### 劣势

1. **性能开销**
   - 每次渲染文本都需要调用系统 API
   - 需要创建临时位图
   - 有 JNI 调用开销（Android）

2. **缓存机制**
   - PPSSPP 实现了文本缓存（TextDrawer::cache_）
   - 相同的文本只渲染一次，后续使用缓存的纹理

## OHOS 平台的实现

### 当前状态

OHOS 平台目前**没有**实现 `TextDrawerOHOS`，所以：

```cpp
#if PPSSPP_PLATFORM(WINDOWS) || PPSSPP_PLATFORM(ANDROID) || PPSSPP_PLATFORM(MAC) || PPSSPP_PLATFORM(IOS)
    drawer = new TextDrawerAndroid(draw);  // OHOS 不在这里
#elif USE_SDL2_TTF
    drawer = new TextDrawerSDL(draw);
#else
    drawer = new TextDrawerUWP(draw);  // OHOS 可能会走这里
#endif
```

### 需要实现的功能

#### 选项 1: 使用完整字体图集（临时方案）

```cpp
// UI/UIAtlas.cpp
#if PPSSPP_PLATFORM(WINDOWS) || PPSSPP_PLATFORM(ANDROID) || PPSSPP_PLATFORM(MAC) || PPSSPP_PLATFORM(IOS)
    // 移除 OHOS，让它使用完整字体
#else
    // OHOS 使用完整字体图集
    LoadAtlasMetadata(font_atlas, "font_atlas.meta");
    fontTexture = CreateTextureFromFile(draw, "font_atlas.zim", ImageFileType::ZIM, false);
#endif
```

**优点**: 简单，立即可用
**缺点**: 
- 包体积增加 ~2MB
- 只支持预定义的字符集
- 不支持 Emoji

#### 选项 2: 实现 TextDrawerOHOS（推荐）

创建 `Common/Render/Text/draw_text_ohos.cpp`:

```cpp
class TextDrawerOHOS : public TextDrawer {
public:
    TextDrawerOHOS(Draw::DrawContext *draw);
    ~TextDrawerOHOS();
    
    void SetOrCreateFont(const FontStyle &style) override;
    bool DrawStringBitmap(...) override;
    
protected:
    void MeasureStringInternal(...) override;
    
private:
    // 通过 NAPI 调用 ArkTS 层的文本渲染
    napi_ref textRendererRef_;
    napi_env env_;
};

bool TextDrawerOHOS::DrawStringBitmap(..., std::string_view str, ...) {
    // 1. 调用 ArkTS 层的文本渲染函数
    napi_value result = CallArkTSTextRenderer(str, fontSize);
    
    // 2. 获取渲染后的位图数据
    // ... 从 ArkTS 获取 PixelMap 数据 ...
    
    // 3. 转换为纹理格式
    // ... 复制像素数据 ...
    
    return true;
}
```

在 ArkTS 层实现文本渲染：

```typescript
// TextRenderer.ets
import { drawing } from '@kit.ArkGraphics2D';

export class TextRenderer {
  static renderText(text: string, fontSize: number): PixelMap {
    // 1. 创建 Canvas
    const canvas = new drawing.Canvas(...);
    
    // 2. 设置字体
    const font = new drawing.Font();
    font.setSize(fontSize);
    
    // 3. 渲染文本
    const textBlob = drawing.TextBlob.makeFromString(text, font);
    canvas.drawTextBlob(textBlob, 0, 0);
    
    // 4. 返回位图
    return canvas.getPixelMap();
  }
}
```

**优点**:
- ✅ 支持所有 Unicode 字符
- ✅ 支持系统字体
- ✅ 包体积小
- ✅ 与其他平台一致

**缺点**:
- 需要实现 NAPI 绑定
- 需要学习 HarmonyOS 的 Canvas API

## 代码位置

### 文本渲染实现
- `Common/Render/Text/draw_text.h` - 基类定义
- `Common/Render/Text/draw_text.cpp` - 通用实现
- `Common/Render/Text/draw_text_android.cpp` - Android 实现
- `Common/Render/Text/draw_text_android.h` - Android 头文件
- `Common/Render/Text/draw_text_win.cpp` - Windows 实现
- `Common/Render/Text/draw_text_cocoa.mm` - macOS/iOS 实现

### Java 端
- `android/src/org/ppsspp/ppsspp/TextRenderer.java` - Android 文本渲染

### 字体图集
- `UI/UIAtlas.cpp` - 图集加载
- `assets/asciifont_atlas.zim` - ASCII 字体图集
- `assets/font_atlas.zim` - 完整字体图集

## 总结

**问题答案**: Android 平台虽然只加载了 `asciifont_atlas.zim`（用于 UI 图标），但所有的文本都是通过 `TextDrawerAndroid` 使用 Android 系统的 Canvas API 动态渲染的，因此可以显示任何 Unicode 字符，包括中文。

**OHOS 建议**:
1. **短期**: 使用完整字体图集 `font_atlas.zim`（移除 OHOS 的 ANDROID 宏）
2. **长期**: 实现 `TextDrawerOHOS`，使用 HarmonyOS 的 Canvas API 动态渲染文本

这样既能保持与其他平台的一致性，又能充分利用系统字体资源。
