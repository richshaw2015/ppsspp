# PPSSPP 鸿蒙适配 - 快速开始指南

## 当前状态

✅ **已完成**: 第三方依赖库集成 (22 个库)  
⏳ **进行中**: Common 和 Core 库集成  
⏳ **待完成**: 编译测试和平台适配

## 快速命令

### 验证依赖库
```bash
cd ohos
./verify_dependencies.sh
```

### 查看集成计划
```bash
cat docs/CORE_INTEGRATION_PLAN.md
cat docs/DEPENDENCIES_INTEGRATION.md
cat docs/INTEGRATION_SUMMARY.md
```

### 尝试编译
```bash
cd ohos
./hvigorw assembleHap
```

## 下一步操作

### 1. 完善源文件列表

#### 方法 A: 手动复制（推荐）
```bash
# 1. 打开主 CMakeLists.txt
open ../CMakeLists.txt  # macOS
# 或
vim ../CMakeLists.txt   # Linux

# 2. 找到 Common 库定义（约第 700 行）
# 搜索: add_library(Common STATIC

# 3. 复制所有源文件到
vim entry/src/main/cpp/common_sources.cmake

# 4. 找到 Core 库定义（约第 2057 行）
# 搜索: add_library(${CoreLibName} ${CoreLinkType}

# 5. 复制所有源文件到
vim entry/src/main/cpp/core_sources.cmake
```

#### 方法 B: 参考 Android（推荐）
```bash
# Android 使用相同的源文件列表
# 查找 Android 的 JNI 配置
find ../android -name "*.mk" -o -name "CMakeLists.txt"

# 参考 Android 的源文件组织
```

### 2. 首次编译

```bash
cd ohos

# 清理之前的构建
./hvigorw clean

# 尝试编译
./hvigorw assembleHap

# 查看错误（如果有）
# 根据错误类型修复：
# - 缺少源文件: 添加到 *_sources.cmake
# - 链接错误: 检查 CMakeLists.txt 的 target_link_libraries
# - 头文件错误: 检查 include_directories
```

### 3. 常见问题处理

#### 问题 1: 找不到头文件
```cmake
# 在 CMakeLists.txt 添加
target_include_directories(Common PRIVATE
    ${PPSSPP_ROOT}/ext/某个库
)
```

#### 问题 2: 符号未定义
```cmake
# 在 CMakeLists.txt 添加
target_link_libraries(Core PRIVATE
    某个缺少的库
)
```

#### 问题 3: 平台 API 不兼容
```cpp
// 使用条件编译
#ifdef OHOS
    // 鸿蒙特定代码
#elif defined(ANDROID)
    // Android 代码
#endif
```

## 项目结构

```
ohos/
├── entry/src/main/
│   ├── cpp/
│   │   ├── CMakeLists.txt          # 主构建配置 ✅
│   │   ├── common_sources.cmake    # Common 源文件 ⏳
│   │   ├── core_sources.cmake      # Core 源文件 ⏳
│   │   ├── napi/                   # NAPI 接口 ✅
│   │   ├── ohos_*.cpp              # 平台适配 ⏳
│   │   └── ...
│   └── ets/                        # TypeScript 代码
├── docs/
│   ├── CORE_INTEGRATION_PLAN.md    # 集成计划 ✅
│   ├── DEPENDENCIES_INTEGRATION.md # 依赖指南 ✅
│   └── INTEGRATION_SUMMARY.md      # 集成总结 ✅
├── verify_dependencies.sh          # 验证脚本 ✅
├── generate_sources.sh             # 生成脚本 ✅
└── QUICK_START.md                  # 本文档 ✅
```

## 关键文件

### 需要编辑的文件
1. `entry/src/main/cpp/common_sources.cmake` - 添加 Common 源文件
2. `entry/src/main/cpp/core_sources.cmake` - 添加 Core 源文件
3. `entry/src/main/cpp/ohos_*.cpp` - 实现平台适配

### 参考文件
1. `../CMakeLists.txt` - 主构建配置（源文件列表）
2. `../ext/CMakeLists.txt` - 第三方库配置
3. `../android/jni/` - Android 平台实现

## 预期时间线

- **今天**: 完成源文件列表提取
- **明天**: 首次编译尝试，修复基础错误
- **3-5 天**: 修复所有编译错误
- **1 周**: 完成基础功能
- **2 周**: 优化和测试

## 获取帮助

### 查看文档
```bash
ls -la docs/
cat docs/INTEGRATION_SUMMARY.md
```

### 检查依赖
```bash
./verify_dependencies.sh
```

### 查看构建日志
```bash
./hvigorw assembleHap 2>&1 | tee build.log
grep -i error build.log
```

## 联系方式

如有问题，请参考：
- 主项目文档: `../README.md`
- 集成计划: `docs/CORE_INTEGRATION_PLAN.md`
- 依赖指南: `docs/DEPENDENCIES_INTEGRATION.md`

---

**提示**: 由于源文件数量庞大（1000+），建议分模块逐步集成，先让基础框架编译通过。
