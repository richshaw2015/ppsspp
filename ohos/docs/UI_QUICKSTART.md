# UI 库集成 - 快速开始

## 编译

```bash
cd ohos
./gradlew clean
./gradlew assembleDebug
```

## 验证

### 1. 检查 UI 库

```bash
ls -lh entry/.cxx/default/default/debug/arm64-v8a/libUI.a
```

预期: ~15-20 MB

### 2. 检查最终库

```bash
ls -lh entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so
```

预期: ~200-220 MB (Debug)

### 3. 检查符号

```bash
nm -D entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep -E "NativeInit|NativeFrame|NativeShutdown"
```

应该看到:
- `NativeInit`
- `NativeInitGraphics`
- `NativeFrame`
- `NativeShutdownGraphics`

## 集成的内容

### UI 库 (50+ 文件)
- ✅ 核心应用 (NativeApp.cpp)
- ✅ 主要屏幕 (MainScreen, EmuScreen, GameScreen, PauseScreen)
- ✅ 设置屏幕 (10+ 屏幕)
- ✅ UI 组件 (Theme, UIAtlas, Background)
- ✅ 调试工具 (ImDebugger)

### 构建配置
- ✅ `ui_sources.cmake` - UI 源文件列表
- ✅ CMakeLists.txt - 添加 UI 库集成
- ✅ whole-archive 链接

## 下一步

### 1. 添加资源文件

```
ohos/entry/src/main/resources/rawfile/
├── assets/
│   ├── ui_atlas.zim
│   ├── ui_atlas.meta
│   └── lang/
│       ├── en_US.ini
│       └── zh_CN.ini
└── fonts/
    └── Roboto-Condensed.ttf
```

### 2. 配置路径

在 `ohos_app.cpp` 中设置:
```cpp
NativeInit(argc, argv,
    "/data/storage/el2/base/files",  // savegame_dir
    "/data/storage/el2/base/files",  // external_dir
    "/data/storage/el2/base/cache"   // cache_dir
);
```

### 3. 测试 UI

运行应用，检查:
- [ ] 主菜单显示
- [ ] 游戏列表显示
- [ ] 设置界面可访问
- [ ] UI 响应触摸

## 故障排查

### 编译失败

**错误**: `undefined reference to NativeInit`

**解决**:
```cmake
# 确保 UI 库在 whole-archive 中
target_link_libraries(ppsspp_ohos
    PRIVATE
    -Wl,--whole-archive
    UI  # 必须包含
    -Wl,--no-whole-archive
)
```

### UI 不显示

**检查**:
1. `NativeInitGraphics` 是否返回 true
2. 资源文件是否存在
3. OpenGL 上下文是否正确初始化

**日志**:
```bash
hdc shell hilog | grep -E "NativeInit|UI"
```

### 崩溃

**常见原因**:
- 资源路径错误
- 图形上下文未初始化
- 内存不足

**调试**:
```bash
hdc shell hilog | grep -E "FATAL|ERROR"
```

## 参考文档

- [UI_INTEGRATION.md](docs/UI_INTEGRATION.md) - 详细说明
- [UI_INTEGRATION_SUMMARY.md](UI_INTEGRATION_SUMMARY.md) - 完成总结
- [OPENGL_IMPLEMENTATION.md](docs/OPENGL_IMPLEMENTATION.md) - OpenGL 实现

## 快速命令

```bash
# 编译
./gradlew assembleDebug

# 安装
./gradlew installDebug

# 查看日志
hdc shell hilog | grep PPSSPP

# 检查库大小
ls -lh entry/.cxx/default/default/debug/arm64-v8a/*.so

# 检查符号
nm -D entry/.cxx/default/default/debug/arm64-v8a/libppsspp_ohos.so | grep Native
```
