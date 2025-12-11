# PPSSPP 鸿蒙适配 - 依赖库集成总结

## 已完成的工作

### 1. 第三方依赖库集成 (ext/)

已成功集成 22 个核心第三方依赖库：

#### 基础库
1. **cpu_features** - CPU 特性检测 (NEON、SSE等)
2. **snappy** - 快速压缩/解压
3. **zlib** - 通用压缩库
4. **gason** - JSON 解析
5. **cityhash** - 哈希函数
6. **xxhash** - 快速哈希

#### 图像处理
7. **libpng17** - PNG 图像加载
8. **basis_universal** - 纹理压缩
9. **xbrz** - 图像缩放算法

#### 文件处理
10. **libzip** - ZIP 文件处理
11. **libchdr** - CHD 文件格式支持

#### 着色器
12. **glslang** - GLSL 着色器编译
13. **SPIRV-Cross** - SPIR-V 着色器转换

#### 音频
14. **minimp3** - MP3 解码
15. **at3_standalone** - ATRAC3 音频解码

#### 脚本和成就
16. **lua** - Lua 脚本支持
17. **rcheevos** - RetroAchievements 支持

#### PSP 特定
18. **kirk** - PSP 加密库
19. **sfmt19937** - 随机数生成器
20. **armips** - ARM 汇编器

#### 其他
21. **udis86** - x86 反汇编器
22. **vma** - Vulkan 内存分配器

### 2. 构建配置

#### CMakeLists.txt 结构
```
ohos/entry/src/main/cpp/CMakeLists.txt
├── 编译宏定义 (OHOS=1, MOBILE_DEVICE=1, etc.)
├── 第三方依赖库 (22 个)
├── Common 库 (待完善)
├── native 库
├── Core 库 (待完善)
└── 最终链接
```

#### 关键配置
- **C++ 标准**: C++17
- **架构**: ARM64
- **图形 API**: OpenGL ES 2.0/3.0 + EGL
- **构建类型**: 静态库 + 共享库

### 3. 文档

已创建以下文档：
- `CORE_INTEGRATION_PLAN.md` - 核心集成计划
- `DEPENDENCIES_INTEGRATION.md` - 依赖库集成完整指南
- `INTEGRATION_SUMMARY.md` - 本文档

### 4. 验证脚本

- `verify_dependencies.sh` - 验证依赖库完整性
- `generate_sources.sh` - 生成源文件列表

## 待完成的工作

### 1. 完善源文件列表

#### Common 库 (~200 个源文件)
需要从主 CMakeLists.txt 提取完整的源文件列表，包括：
- GPU 抽象层 (OpenGL/Vulkan)
- 网络/HTTP
- UI 框架
- 文件系统
- 数学库
- 渲染系统
- 输入系统
- 线程管理

**操作步骤**:
1. 打开 `../CMakeLists.txt`
2. 找到 `add_library(Common STATIC ...)`
3. 复制所有源文件到 `common_sources.cmake`
4. 替换路径前缀为 `${PPSSPP_ROOT}/`

#### Core 库 (~1000 个源文件)
需要提取完整的 Core 库源文件，包括：
- MIPS CPU 模拟 (解释器 + JIT)
- GPU 模拟 (GLES + Vulkan)
- HLE (约 100 个 sceXXX.cpp 文件)
- 文件加载器
- 调试器
- 对话框系统

**操作步骤**:
1. 打开 `../CMakeLists.txt`
2. 找到 `add_library(${CoreLibName} ${CoreLinkType} ...)`
3. 复制所有源文件到 `core_sources.cmake`
4. 注意 Android 使用 `ppsspp_jni`，其他平台使用 `Core`

### 2. 平台适配

需要创建鸿蒙特定的适配文件：

#### 文件系统适配
```cpp
// ohos/entry/src/main/cpp/ohos_storage.cpp
// 替代 Common/File/AndroidStorage.cpp
// 实现鸿蒙的文件访问 API

// ohos/entry/src/main/cpp/ohos_content_uri.cpp
// 替代 Common/File/AndroidContentURI.cpp
// 实现鸿蒙的 URI 处理
```

#### 文本渲染适配
```cpp
// ohos/entry/src/main/cpp/ohos_text_render.cpp
// 替代 Common/Render/Text/draw_text_android.cpp
// 使用鸿蒙的文本渲染 API
```

#### 音频适配
```cpp
// ohos/entry/src/main/cpp/ohos_audio.cpp (已存在，需完善)
// 实现完整的音频后端
```

### 3. 编译和调试

#### 预期编译问题
1. **缺少头文件** - 需要添加更多 include 目录
2. **符号未定义** - 需要链接更多库
3. **平台 API 差异** - 需要条件编译或适配
4. **内存对齐问题** - ARM64 特定问题

#### 调试步骤
```bash
# 1. 尝试编译
cd ohos
./hvigorw assembleHap

# 2. 查看错误日志
# 根据错误类型逐个修复

# 3. 常见错误处理
# - 缺少源文件: 添加到 *_sources.cmake
# - 链接错误: 检查 target_link_libraries
# - 头文件错误: 检查 include_directories
```

### 4. 性能优化

#### 编译优化
```cmake
# Release 模式优化选项
target_compile_options(Core PRIVATE
    "$<$<CONFIG:Release>:-O3>"
    "$<$<CONFIG:Release>:-ffast-math>"
    "$<$<CONFIG:Release>:-ffunction-sections>"
    "$<$<CONFIG:Release>:-fdata-sections>"
)

# 链接时优化
target_link_options(ppsspp_ohos PRIVATE
    "$<$<CONFIG:Release>:-Wl,--gc-sections>"
    "$<$<CONFIG:Release>:-Wl,--strip-all>"
)
```

#### 库大小优化
- 移除未使用的功能 (Vulkan、Discord 等)
- 使用 LTO (Link Time Optimization)
- 压缩资源文件

## 下一步行动计划

### 短期 (1-2 天)
1. ✅ 完成第三方依赖库集成
2. ⏳ 提取完整的 Common 源文件列表
3. ⏳ 提取完整的 Core 源文件列表
4. ⏳ 首次编译尝试

### 中期 (3-5 天)
1. 修复编译错误
2. 实现平台适配代码
3. 完成基础功能测试
4. 优化编译配置

### 长期 (1-2 周)
1. 性能优化
2. 功能完善
3. 稳定性测试
4. 文档完善

## 参考资源

### 主要参考
- **主 CMakeLists.txt** - 完整的构建配置
- **Android 平台** - 最接近的移动平台实现
- **iOS 平台** - 另一个移动平台参考

### 关键文件位置
```
../CMakeLists.txt                    # 主构建配置
../ext/CMakeLists.txt                # 第三方库配置
../android/jni/                      # Android 平台代码
../Common/                           # 通用库
../Core/                             # 核心库
../GPU/                              # GPU 模拟
```

### 有用的命令
```bash
# 查看 Common 库源文件数量
grep -c "\.cpp" ../CMakeLists.txt | head -1

# 查找所有 HLE 文件
find ../Core/HLE -name "*.cpp" | wc -l

# 检查依赖库
ls -la ../ext/

# 查看编译日志
./hvigorw assembleHap 2>&1 | tee build.log
```

## 总结

已完成第三方依赖库的集成框架，建立了完整的构建配置结构。下一步需要：
1. 提取完整的源文件列表（这是最耗时的部分）
2. 进行首次编译尝试
3. 根据编译错误逐步完善

预计完整集成需要 1-2 周时间，主要挑战在于：
- 源文件数量庞大（1000+ 个）
- 平台 API 差异需要适配
- 编译错误可能较多

建议采用渐进式方法：先让基础框架编译通过，再逐步添加功能模块。
