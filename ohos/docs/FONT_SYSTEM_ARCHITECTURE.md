# PPSSPP 多层字体系统架构分析

## 为什么使用多层字体系统？

PPSSPP 作为一个 PSP 模拟器，需要同时处理两个完全不同的渲染环境：
1. **PSP 游戏内容的模拟渲染** - 需要完全兼容 PSP 原生字体系统
2. **模拟器 UI 界面渲染** - 需要现代化的高效字体渲染

这种双重需求导致了多层字体系统的设计。

## 三层字体系统详解

### 第一层：PSP 原生字体系统 (PGF)

#### 技术特点
```cpp
// Core/Font/PGF.cpp - PSP 原生字体格式
class PGF {
    // PGF 是 PSP 专用的压缩字体格式
    // 支持位图字体和矢量字体混合
    // 包含字符映射表和压缩的字形数据
};
```

#### 设计目标
- **完全兼容性**: 100% 模拟 PSP 原生字体行为
- **内存效率**: PSP 只有 32MB 内存，字体必须高度压缩
- **硬件优化**: 针对 PSP 的 MIPS 处理器和 GPU 优化

#### 技术实现
```cpp
// 字体注册表 - 模拟 PSP flash0:/font/ 目录
static const FontRegistryEntry fontRegistry[] = {
    // 中文字体 (为中文翻译游戏添加)
    { 0x288, 0x288, 0x2000, 0x2000, 0, 0, FONT_FAMILY_SANS_SERIF, 
      FONT_STYLE_DB, 0, FONT_LANGUAGE_CHINESE, 0, 1, 
      "zh_gb.pgf", "FTT-NewRodin Pro DB", 0, 0, 1581700, 145844, true },
    
    // 日文字体
    { 0x288, 0x288, 0x2000, 0x2000, 0, 0, FONT_FAMILY_SANS_SERIF, 
      FONT_STYLE_DB, 0, FONT_LANGUAGE_JAPANESE, 0, 1, 
      "jpn0.pgf", "FTT-NewRodin Pro DB", 0, 0, 1581700, 145844 },
};
```

#### 优势
- **精确模拟**: 游戏字体显示与真实 PSP 完全一致
- **压缩效率**: PGF 格式针对游戏字体优化，压缩率高
- **字符集完整**: 包含游戏需要的所有字符

#### 劣势
- **格式封闭**: PGF 是 Sony 专有格式，难以修改
- **工具链复杂**: 需要专门的工具来创建和编辑 PGF 字体
- **灵活性差**: 无法动态添加字符或调整样式

### 第二层：字体图集系统 (Font Atlas)

#### 技术特点
```cpp
// Common/Render/TextureAtlas.cpp - 字体图集实现
class AtlasFont {
    AtlasChar* charData;        // 字符数据数组
    AtlasCharRange* ranges;     // 字符范围映射
    float height, ascend;       // 字体度量信息
    
    const AtlasChar* getChar(int utf32) const {
        // 快速字符查找 - O(log n) 复杂度
        for (int i = 0; i < numRanges; i++) {
            if (utf32 >= ranges[i].start && utf32 < ranges[i].end) {
                return &charData[ranges[i].charIndex + (utf32 - ranges[i].start)];
            }
        }
        return nullptr;
    }
};
```

#### 设计目标
- **渲染性能**: 所有字符预渲染到单个纹理中
- **内存局部性**: 字符数据连续存储，缓存友好
- **GPU 优化**: 减少纹理切换，提高批处理效率

#### 生成流程
```bash
# font_atlasscript.txt 配置
2048                                    # 图集大小
font UBUNTU24 assets/Roboto_Condensed-Regular.ttf UWERS 34 -2
font UBUNTU24 assets/chinese-font.ttf UWEhkcRGHKVTeS一二三四五六七八九十中文测试 30 0

# 生成过程
./build_fontatlas.sh
├── 1. 解析字体文件 (TTF/OTF)
├── 2. 渲染指定字符到位图
├── 3. 打包到单个纹理图集
├── 4. 生成字符映射数据
└── 5. 输出 .zim 和 .meta 文件
```

#### 优势
- **极高性能**: 所有字符在一个纹理中，GPU 批处理效率最高
- **内存效率**: 预计算字符布局，运行时零分配
- **缓存友好**: 字符数据紧密排列，CPU 缓存命中率高
- **跨平台**: 标准的纹理格式，所有 GPU 都支持

#### 劣势
- **静态性**: 字符集必须预先确定，无法动态添加
- **内存占用**: 大字符集会产生很大的纹理
- **字体大小固定**: 每个大小需要单独的图集
- **构建时间**: 大字符集的图集生成耗时较长

### 第三层：动态文本渲染系统 (TextDrawer)

#### 技术特点
```cpp
// Common/Render/Text/draw_text.cpp - 动态文本渲染
class TextDrawer {
private:
    std::map<FontKey, std::unique_ptr<Font>> cache_;  // 字体缓存
    float dpiScale_;                                  // DPI 缩放
    
public:
    void DrawText(DrawBuffer &target, const char *text, float x, float y, 
                  uint32_t color, int align = ALIGN_TOPLEFT) {
        // 动态渲染文本到目标缓冲区
        // 支持复杂布局、换行、对齐等
    }
    
    void ClearCache() {
        // DPI 变化时清理缓存
        INFO_LOG(Log::G3D, "DPI Scale changed - wiping font cache (%d items)", 
                 (int)cache_.size());
        cache_.clear();
    }
};
```

#### 设计目标
- **灵活性**: 支持任意字体、大小、样式
- **质量**: 高质量的文本渲染和布局
- **适应性**: 自动适应不同 DPI 和屏幕尺寸

#### 技术实现
```cpp
// 多级缓存策略
class FontCache {
    // L1: 字符纹理缓存 (GPU 内存)
    std::map<CharKey, TextureRef> glyphTextures_;
    
    // L2: 字体实例缓存 (系统内存)
    std::map<FontKey, FontInstance> fontInstances_;
    
    // L3: 字体文件缓存 (磁盘缓存)
    std::map<std::string, FontData> fontFiles_;
};
```

#### 优势
- **最高灵活性**: 支持任意字体和样式组合
- **质量最佳**: 使用系统字体渲染引擎，质量最高
- **功能完整**: 支持复杂文本布局、国际化等
- **适应性强**: 自动适应不同设备和分辨率

#### 劣势
- **性能开销**: 动态渲染比预渲染慢
- **内存波动**: 缓存管理复杂，内存使用不稳定
- **依赖性**: 依赖系统字体库和渲染引擎

## 技术对比分析

### 性能对比

| 技术 | 渲染速度 | 内存使用 | GPU 效率 | CPU 开销 |
|------|----------|----------|----------|----------|
| PGF 字体 | 快 | 低 | 高 | 低 |
| 字体图集 | 极快 | 中等 | 极高 | 极低 |
| 动态渲染 | 慢 | 高 | 低 | 高 |

### 适用场景

#### PGF 字体系统
```cpp
// 适用于：PSP 游戏内容渲染
void PSPGame::RenderText() {
    // 游戏调用 sceFont API
    sceFontGetCharGlyphImage(fontHandle, charCode, &glyphImage);
    // 必须与真实 PSP 行为完全一致
}
```

#### 字体图集系统
```cpp
// 适用于：高频率的 UI 文本渲染
void GameUI::RenderHUD() {
    // 游戏 HUD、菜单、按钮文字
    drawBuffer.DrawText(UBUNTU24, "Score: 12345", x, y, color);
    // 需要极高的渲染性能
}
```

#### 动态渲染系统
```cpp
// 适用于：模拟器界面和复杂文本
void EmulatorUI::RenderSettings() {
    // 设置界面、文件浏览器、调试信息
    textDrawer->DrawText("游戏设置", x, y, color, ALIGN_CENTER);
    // 需要支持多语言和复杂布局
}
```

## 系统集成策略

### 字体回退机制
```cpp
class FontFallbackSystem {
    std::vector<FontSource> sources_ = {
        FontSource::ATLAS,      // 优先使用图集 (最快)
        FontSource::SYSTEM,     // 回退到系统字体
        FontSource::EMBEDDED    // 最后使用内嵌字体
    };
    
    const AtlasChar* GetChar(uint32_t codepoint) {
        for (auto source : sources_) {
            if (auto* ch = TryGetChar(source, codepoint)) {
                return ch;
            }
        }
        return GetReplacementChar(); // 显示 '?' 或 '□'
    }
};
```

### 性能优化策略

#### 1. 分层缓存
```cpp
// 三级缓存系统
L1_Cache: GPU 纹理缓存 (最快访问)
L2_Cache: 系统内存缓存 (快速访问)  
L3_Cache: 磁盘文件缓存 (慢速访问)
```

#### 2. 预加载策略
```cpp
void PreloadCommonChars() {
    // 预加载常用字符到图集
    std::string commonChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string commonChinese = "的一是在不了有和人这中大为上个国我以要他时来用们生到作地于出就分对成会可主发年动同工也能下过子说产种面而方后多定行学法所民得经十三之进着等部度家电力里如水化高自二理起小物现实加量都两体制机当使点从业本去把性好应开它合还因由其些然前外天政四日那社义事平形相全表间样与关各重新线内数正心反你明看原又么利比或但质气第向道命此变条只没结解问意建月公无系军很情者最立代想已通并提直题党程展五果料象员革位入常文总次品式活设及管特件长求老头基资边流路级少图山统接知较将组见计别她手角期根论运农指几九区强放决西被干做必战先回则任取据处队南给色光门即保治北造百规热领七海口东导器压志世金增争济阶油思术极交受联什认六共权收证改清己美再采转更单风切打白教速花带安场身车例真务具万每目至达走积示议声报斗完类八离华名确才科张信马节话米整空元况今集温传土许步群广石记需段研界拉林律叫且究观越织装影算低持音众书布复容儿须际商非验连断深难近矿千周委素技备半办青省列习响约支般史感劳便团往酸历市克何除消构府称太准精值号率族维划选标写存候毛亲快效斯院查江型眼王按格养易置派层片始却专状育厂京识适属圆包火住调满县局照参红细引听该铁价严龙飞";
    
    for (char32_t ch : UTF8ToUTF32(commonChars + commonChinese)) {
        PreloadCharToAtlas(ch);
    }
}
```

#### 3. 内存管理
```cpp
class FontMemoryManager {
    void OnMemoryPressure() {
        // 内存压力时的清理策略
        ClearLRUCache();           // 清理最少使用的缓存
        CompactAtlasTextures();    // 压缩图集纹理
        UnloadUnusedFonts();       // 卸载未使用的字体
    }
    
    void OnDPIChange(float newDPI) {
        // DPI 变化时重建缓存
        InvalidateAllCaches();
        RebuildAtlasForNewDPI(newDPI);
    }
};
```

## 总结

PPSSPP 的多层字体系统是一个精心设计的架构，每一层都有其特定的用途和优势：

1. **PGF 层**：确保游戏兼容性，这是模拟器的核心要求
2. **图集层**：提供极致性能，满足实时渲染需求  
3. **动态层**：提供最大灵活性，支持现代 UI 需求

这种设计让 PPSSPP 能够在保证完美游戏兼容性的同时，提供流畅的用户体验和现代化的界面。每种技术都在其最适合的场景中发挥作用，形成了一个高效、灵活、兼容的字体渲染系统。