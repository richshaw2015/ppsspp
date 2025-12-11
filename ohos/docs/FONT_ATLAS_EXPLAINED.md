# 字体图集 (Font Atlas) 详解

## 什么是字体图集？

字体图集是一种将多个字符预先渲染到单个纹理中的技术。想象一下，它就像是一张"字符贴纸册"，把所有需要的字符都画在一张大图片上，然后在需要显示文字时，从这张图片中"剪切"出对应的字符。

## 为什么需要字体图集？

### 传统方式的问题
```cpp
// 传统的动态字体渲染 (每次都要重新生成)
void RenderText_Traditional(const char* text) {
    for (char c : text) {
        // 1. 加载字体文件 (慢)
        // 2. 渲染字符到位图 (慢)
        // 3. 创建GPU纹理 (慢)
        // 4. 上传到GPU (慢)
        // 5. 绘制到屏幕 (快)
        RenderCharacter(c);  // 每个字符都要重复1-4步骤！
    }
}
```

### 字体图集的优势
```cpp
// 字体图集方式 (预先准备好所有字符)
void RenderText_Atlas(const char* text) {
    BindAtlasTexture();  // 只需要绑定一次纹理
    for (char c : text) {
        // 1. 查找字符在图集中的位置 (极快)
        // 2. 直接绘制纹理区域 (极快)
        DrawAtlasChar(c);  // 所有字符共享同一个纹理！
    }
}
```

## 字体图集的数据结构

### 核心数据结构
```cpp
// 单个字符的信息
struct AtlasChar {
    // 纹理坐标 (在图集中的位置)
    float sx, sy;    // 起始坐标 (左上角)
    float ex, ey;    // 结束坐标 (右下角)
    
    // 渲染偏移 (字符相对于基线的位置)
    float ox, oy;    // 偏移量
    
    // 字符宽度 (光标移动距离)
    float wx;        // 字符宽度
    
    // 像素尺寸
    unsigned short pw, ph;  // 像素宽度和高度
};

// 字符范围 (用于快速查找)
struct AtlasCharRange {
    int start;          // 范围起始 (如 'A' = 65)
    int end;            // 范围结束 (如 'Z' = 90)
    int result_index;   // 在charData数组中的起始索引
};

// 字体图集
struct AtlasFont {
    float height;                    // 字体高度
    float ascend;                    // 上升高度
    const AtlasChar *charData;       // 所有字符数据
    const AtlasCharRange *ranges;    // 字符范围数组
    int numRanges;                   // 范围数量
    int numChars;                    // 字符总数
    char name[32];                   // 字体名称
};
```

## 图集生成过程

### 1. 配置阶段 (font_atlasscript.txt)
```
2048                                    # 图集大小 2048x2048 像素
font UBUNTU24 assets/Roboto_Condensed-Regular.ttf UWERS 34 -2
font UBUNTU24 assets/chinese-font.ttf UWEhkcRGHKVTeS一二三四五六七八九十中文测试 30 0
```

参数解释：
- `2048`: 图集纹理的尺寸 (2048x2048 像素)
- `UBUNTU24`: 字体标识符
- `assets/Roboto_Condensed-Regular.ttf`: 字体文件路径
- `UWERS一二三四五六七八九十中文测试`: 要包含的字符集
- `34`: 字体大小 (像素)
- `-2`: Y轴偏移调整

### 2. 字符渲染阶段
```cpp
// atlastool.cpp 中的核心逻辑
void GenerateAtlas() {
    // 1. 加载字体文件
    FT_Face face = LoadFontFile("assets/chinese-font.ttf");
    
    // 2. 为每个字符生成位图
    std::vector<CharBitmap> charBitmaps;
    for (char32_t c : characterSet) {
        // 设置字体大小
        FT_Set_Pixel_Sizes(face, 0, fontSize);
        
        // 渲染字符
        FT_Load_Char(face, c, FT_LOAD_RENDER);
        
        // 保存位图数据
        CharBitmap bitmap = {
            .character = c,
            .width = face->glyph->bitmap.width,
            .height = face->glyph->bitmap.rows,
            .data = face->glyph->bitmap.buffer,
            .offsetX = face->glyph->bitmap_left,
            .offsetY = face->glyph->bitmap_top,
            .advanceX = face->glyph->advance.x >> 6
        };
        charBitmaps.push_back(bitmap);
    }
    
    // 3. 打包到图集纹理
    PackBitmapsToAtlas(charBitmaps);
}
```

### 3. 图集打包阶段
```cpp
void PackBitmapsToAtlas(std::vector<CharBitmap>& bitmaps) {
    // 创建2048x2048的图集纹理
    uint8_t* atlasTexture = new uint8_t[2048 * 2048 * 4]; // RGBA
    
    int currentX = 0, currentY = 0;
    int rowHeight = 0;
    
    for (auto& bitmap : bitmaps) {
        // 检查是否需要换行
        if (currentX + bitmap.width > 2048) {
            currentX = 0;
            currentY += rowHeight;
            rowHeight = 0;
        }
        
        // 复制位图数据到图集
        CopyBitmapToAtlas(bitmap, atlasTexture, currentX, currentY);
        
        // 记录字符在图集中的位置
        AtlasChar atlasChar = {
            .sx = (float)currentX / 2048.0f,
            .sy = (float)currentY / 2048.0f,
            .ex = (float)(currentX + bitmap.width) / 2048.0f,
            .ey = (float)(currentY + bitmap.height) / 2048.0f,
            .ox = (float)bitmap.offsetX,
            .oy = (float)bitmap.offsetY,
            .wx = (float)bitmap.advanceX,
            .pw = bitmap.width,
            .ph = bitmap.height
        };
        
        // 更新位置
        currentX += bitmap.width + 1; // +1 为间距
        rowHeight = std::max(rowHeight, bitmap.height);
    }
}
```

## 字符查找机制

### 快速查找算法
```cpp
const AtlasChar* AtlasFont::getChar(int utf32) const {
    // 使用范围查找 - O(log n) 复杂度
    for (int i = 0; i < numRanges; i++) {
        if (utf32 >= ranges[i].start && utf32 < ranges[i].end) {
            // 计算在charData数组中的索引
            int index = ranges[i].result_index + (utf32 - ranges[i].start);
            const AtlasChar* c = &charData[index];
            
            // 检查字符是否有效 (宽度和高度不为0)
            if (c->ex == 0 && c->ey == 0)
                return nullptr;  // 字符不存在
            else
                return c;        // 返回字符信息
        }
    }
    return nullptr;  // 未找到字符
}
```

### 范围优化示例
```cpp
// 字符范围表 - 优化查找性能
AtlasCharRange ranges[] = {
    {0x0020, 0x007F, 0},    // ASCII: 空格到DEL (95个字符)
    {0x4E00, 0x9FFF, 95},   // 中文: 常用汉字 (约20000个字符)
    {0x3040, 0x309F, 20095} // 日文: 平假名 (96个字符)
};

// 查找字符 '中' (Unicode: 0x4E2D)
// 1. 检查范围0: 0x4E2D不在[0x0020, 0x007F)
// 2. 检查范围1: 0x4E2D在[0x4E00, 0x9FFF) ✓
// 3. 计算索引: 95 + (0x4E2D - 0x4E00) = 95 + 45 = 140
// 4. 返回 charData[140]
```

## 渲染过程详解

### GPU渲染流程
```cpp
void DrawText(const char* text, float x, float y) {
    // 1. 绑定图集纹理 (只需要一次)
    glBindTexture(GL_TEXTURE_2D, fontAtlasTexture);
    
    // 2. 准备顶点数据
    std::vector<Vertex> vertices;
    float currentX = x;
    
    for (UTF8 utf(text); !utf.end(); ) {
        uint32_t codepoint = utf.next();
        
        // 3. 查找字符信息
        const AtlasChar* ch = atlas->getChar(codepoint);
        if (!ch) continue;
        
        // 4. 计算屏幕坐标
        float x1 = currentX + ch->ox;
        float y1 = y + ch->oy;
        float x2 = x1 + ch->pw;
        float y2 = y1 + ch->ph;
        
        // 5. 添加四边形顶点 (2个三角形)
        vertices.push_back({x1, y1, ch->sx, ch->sy}); // 左上
        vertices.push_back({x2, y1, ch->ex, ch->sy}); // 右上
        vertices.push_back({x1, y2, ch->sx, ch->ey}); // 左下
        vertices.push_back({x2, y2, ch->ex, ch->ey}); // 右下
        
        // 6. 移动光标
        currentX += ch->wx;
    }
    
    // 7. 一次性渲染所有字符 (批处理)
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}
```

## 内存布局示意

### 图集纹理布局
```
图集纹理 (2048x2048 像素):
┌─────────────────────────────────────────────────────────────┐
│ A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T │ ← ASCII字符
├─────────────────────────────────────────────────────────────┤
│ U  V  W  X  Y  Z  a  b  c  d  e  f  g  h  i  j  k  l  m  n │
├─────────────────────────────────────────────────────────────┤
│ o  p  q  r  s  t  u  v  w  x  y  z  0  1  2  3  4  5  6  7 │
├─────────────────────────────────────────────────────────────┤
│ 8  9  !  @  #  $  %  ^  &  *  (  )  -  +  =  [  ]  {  }  | │
├─────────────────────────────────────────────────────────────┤
│ 一 二 三 四 五 六 七 八 九 十 中 文 字 体 测 试 显 示 效 果 │ ← 中文字符
├─────────────────────────────────────────────────────────────┤
│ 的 一 是 在 不 了 有 和 人 这 大 为 上 个 国 我 以 要 他 时 │
├─────────────────────────────────────────────────────────────┤
│ ... 更多中文字符 ...                                        │
└─────────────────────────────────────────────────────────────┘
```

### 字符数据数组
```cpp
// charData 数组布局
AtlasChar charData[] = {
    // ASCII 字符 (索引 0-94)
    {0.000f, 0.000f, 0.020f, 0.040f, 2.0f, 24.0f, 12.0f, 10, 32}, // 空格
    {0.020f, 0.000f, 0.035f, 0.040f, 1.0f, 24.0f, 8.0f,  15, 32}, // !
    // ... 更多ASCII字符
    
    // 中文字符 (索引 95-20094)
    {0.000f, 0.200f, 0.030f, 0.240f, 0.0f, 28.0f, 30.0f, 30, 32}, // 一
    {0.030f, 0.200f, 0.060f, 0.240f, 0.0f, 28.0f, 30.0f, 30, 32}, // 二
    // ... 更多中文字符
};

// ranges 数组
AtlasCharRange ranges[] = {
    {0x0020, 0x007F, 0},    // ASCII: 从索引0开始
    {0x4E00, 0x9FFF, 95},   // 中文: 从索引95开始
};
```

## 性能优势分析

### 传统方式 vs 图集方式

| 操作 | 传统方式 | 图集方式 | 性能提升 |
|------|----------|----------|----------|
| 纹理绑定 | 每字符1次 | 整个文本1次 | 100-1000x |
| GPU状态切换 | 频繁 | 极少 | 50-100x |
| 内存访问 | 随机访问 | 连续访问 | 5-10x |
| 缓存命中率 | 低 | 高 | 3-5x |
| 批处理效率 | 无 | 最优 | 10-50x |

### 实际性能数据
```cpp
// 渲染1000个字符的性能对比
Traditional Rendering:
- 纹理绑定: 1000次
- Draw Call: 1000次
- 总时间: ~50ms

Atlas Rendering:
- 纹理绑定: 1次
- Draw Call: 1次 (批处理)
- 总时间: ~0.5ms

性能提升: 100倍！
```

## 图集的限制和解决方案

### 主要限制
1. **静态字符集**: 只能显示预先包含的字符
2. **内存占用**: 大字符集需要大纹理
3. **字体大小固定**: 每个大小需要单独图集

### 解决方案
```cpp
// 1. 多图集系统
class MultiAtlasFont {
    std::vector<AtlasFont> atlases;  // 多个图集
    
    const AtlasChar* getChar(int utf32) {
        // 依次在各个图集中查找
        for (auto& atlas : atlases) {
            if (auto* ch = atlas.getChar(utf32)) {
                return ch;
            }
        }
        return nullptr;  // 回退到动态渲染
    }
};

// 2. 动态图集扩展
class DynamicAtlas {
    AtlasFont staticAtlas;      // 静态图集 (常用字符)
    TextureCache dynamicCache;  // 动态缓存 (罕见字符)
    
    const AtlasChar* getChar(int utf32) {
        // 先查静态图集
        if (auto* ch = staticAtlas.getChar(utf32)) {
            return ch;
        }
        
        // 再查动态缓存
        if (auto* ch = dynamicCache.getChar(utf32)) {
            return ch;
        }
        
        // 动态生成并缓存
        return dynamicCache.generateAndCache(utf32);
    }
};
```

## 总结

字体图集是一种"空间换时间"的优化技术：

**优势**:
- 🚀 **极致性能**: 渲染速度提升100倍以上
- 💾 **GPU友好**: 减少纹理切换和状态变更
- 🔄 **批处理优化**: 所有字符一次性渲染
- 📱 **移动设备友好**: 减少GPU功耗

**代价**:
- 💿 **内存占用**: 需要预分配纹理内存
- 🔒 **灵活性限制**: 字符集需要预先确定
- ⚙️ **构建复杂性**: 需要专门的工具链

在PPSSPP这样的实时应用中，字体图集是UI渲染性能的关键技术，它让界面能够流畅响应，同时保持高质量的文字显示效果。