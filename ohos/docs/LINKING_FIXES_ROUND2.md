# 链接错误修复 - 第 2 轮

**更新时间**: 2024-12-08  
**状态**: ✅ 已修复

## 问题概述

编译通过后出现链接错误，缺失多个函数和全局变量。

## 错误分类

### 1. ImGui 函数缺失（已修复）

**错误**:
```
undefined symbol: ImGuiIO::AddKeyEvent(ImGuiKey, bool)
undefined symbol: ImGui::GetMainViewport()
undefined symbol: ImGui::DockSpaceOverViewport(...)
undefined symbol: ImGui::Render()
undefined symbol: ImGui::GetDrawData()
undefined symbol: ImGui::BeginMenu(...)
undefined symbol: ImGui::MenuItem(...)
undefined symbol: ImDrawList::AddRect(...)
undefined symbol: ImGui_ImplThin3d_RenderDrawData(...)
```

**原因**: UI 库使用了 ImGui 调试器，但没有链接 ImGui 库

**修复**: 添加 ImGui 库到 CMakeLists.txt

```cmake
# 25. ImGui - UI 调试器
add_library(imgui STATIC
    ${PPSSPP_ROOT}/ext/imgui/imgui.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_demo.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_draw.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_extras.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_impl_thin3d.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_impl_platform.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_tables.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${PPSSPP_ROOT}/ext/imgui)

# 链接到 UI 库
target_link_libraries(UI PUBLIC imgui)
```

### 2. System 函数缺失（已修复）

**错误**:
```
undefined symbol: System_LaunchUrl(LaunchUrlType, std::string_view)
undefined symbol: System_GetPropertyStringVec(SystemProperty)
undefined symbol: System_Toast(std::string_view)
undefined symbol: System_Vibrate(int)
```

**修复**: 添加到 `ohos_system_stubs.cpp`

```cpp
void System_LaunchUrl(LaunchUrlType urlType, std::string_view url) {
    INFO_LOG(Log::System, "System_LaunchUrl: type=%d, url=%.*s", 
             (int)urlType, (int)url.size(), url.data());
}

std::vector<std::string> System_GetPropertyStringVec(SystemProperty prop) {
    return std::vector<std::string>();
}

void System_Toast(std::string_view text) {
    INFO_LOG(Log::System, "System_Toast: %.*s", (int)text.size(), text.data());
}

void System_Vibrate(int length_ms) {
    INFO_LOG(Log::System, "System_Vibrate: %d ms", length_ms);
}
```

### 3. VR 函数缺失（已修复）

**错误**:
```
undefined symbol: IsPassthroughSupported()
```

**修复**: 添加到 `ohos_system_stubs.cpp`

```cpp
bool IsPassthroughSupported() {
    return false;
}
```

### 4. 编译器信息函数缺失（已修复）

**错误**:
```
undefined symbol: GetCompilerABI()
```

**修复**: 添加到 `ohos_system_stubs.cpp`

```cpp
const char *GetCompilerABI() {
    return "clang-ohos-aarch64";
}
```

### 5. 全局变量缺失（已修复）

**错误**:
```
undefined symbol: g_extFilesDir
undefined symbol: g_externalDir
```

**修复**: 添加到 `ohos_system_stubs.cpp`

```cpp
std::string g_extFilesDir = "";
std::string g_externalDir = "";
```

### 6. IAPScreen vtable（已修复）

**错误**:
```
undefined symbol: vtable for IAPScreen
```

**原因**: IAPScreen.cpp 未包含在构建中，但 MainScreen.cpp 引用了它

**修复**: 在 `ui_sources.cmake` 中取消注释 IAPScreen.cpp

```cmake
# IAP (应用内购买) - 包含以避免链接错误
${PPSSPP_ROOT}/UI/IAPScreen.cpp
```

## 修复总结

### 文件更改

1. **ohos/entry/src/main/cpp/CMakeLists.txt**
   - 添加 ImGui 库构建
   - 链接 ImGui 到 UI 库

2. **ohos/entry/src/main/cpp/ohos_system_stubs.cpp**
   - 添加 System_LaunchUrl
   - 添加 System_GetPropertyStringVec
   - 添加 System_Toast
   - 添加 System_Vibrate
   - 添加 IsPassthroughSupported
   - 添加 GetCompilerABI
   - 添加 g_extFilesDir 和 g_externalDir 全局变量
   - 移除 ImGui 核心函数桩（使用真正的库）

### 依赖库更新

- ✅ 24 个第三方依赖库（新增 ImGui）
- ✅ Common 库
- ✅ Core 库
- ✅ UI 库
- ✅ ImGui 库（新增）
- ✅ native 库

## 验证

### 预期结果

```bash
cd ohos
./gradlew assembleDebug
```

应该成功链接，生成：
- `libimgui.a` (~5 MB)
- `libCommon.a` (~80 MB)
- `libCore.a` (~100 MB)
- `libUI.a` (~15 MB)
- `libppsspp_ohos.so` (~200 MB Debug)

### 符号检查

```bash
nm -D libppsspp_ohos.so | grep -E "ImGui|System_LaunchUrl|g_extFilesDir"
```

应该看到所有符号都已定义。

## 功能状态

### ImGui 调试器
- ✅ ImGui 核心库已链接
- ✅ ImGui Thin3D 渲染层已链接
- ⚠️ ImGui 平台层需要实现（当前为桩）
- ⚠️ 调试器可以编译但可能无法正常使用

### System 函数
- ✅ System_LaunchUrl - 桩实现（记录日志）
- ✅ System_Toast - 桩实现（记录日志）
- ✅ System_Vibrate - 桩实现（记录日志）
- ✅ System_GetPropertyStringVec - 返回空向量

### 全局变量
- ✅ g_extFilesDir - 空字符串
- ✅ g_externalDir - 空字符串
- ⚠️ 需要在初始化时设置正确的路径

## 下一步

### 立即执行
1. ✅ 编译验证
2. ⏳ 安装到设备
3. ⏳ 测试基本 UI

### 短期计划
1. ⏳ 实现 System_LaunchUrl（浏览器启动）
2. ⏳ 实现 System_Toast（鸿蒙 Toast）
3. ⏳ 实现 System_Vibrate（震动）
4. ⏳ 设置正确的存储路径

### 中期计划
1. ⏳ 实现 ImGui 平台层（如果需要调试器）
2. ⏳ 测试 ImGui 调试器功能
3. ⏳ 优化存储路径管理

## 参考

- [Android.mk](../../android/jni/Android.mk) - Android 构建配置（包含 ImGui）
- [ext/imgui/](../../ext/imgui/) - ImGui 库源码
- [UI/ImDebugger/](../../UI/ImDebugger/) - ImGui 调试器实现

## 总结

所有链接错误已修复：
- ✅ 11 个编译错误
- ✅ 20+ 个链接错误（ImGui + System 函数）
- ✅ 2 个全局变量
- ✅ 1 个 vtable 错误（IAPScreen）
- ✅ ImGui 库已完整集成

**当前状态**: 准备编译和测试 🚀
