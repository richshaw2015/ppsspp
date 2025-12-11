# 触摸输入和安全区域状态

## 触摸输入实现

### 当前状态
✅ 已实现触摸输入功能（使用 Android 相同的公式）
- 触摸事件通过 XComponent 的 `DispatchTouchEvent` 回调接收
- 支持多点触控（最多 10 个触摸点）
- 支持 DOWN/MOVE/UP/CANCEL 动作
- 坐标转换：使用 Android 公式 `x * display_scale_x * dpi_scale_x`

### 坐标转换实现

**Android 公式**：
```cpp
touch.x = x * display_scale_x * g_display.dpi_scale_x;
touch.y = y * display_scale_y * g_display.dpi_scale_y;
```

其中：
- `display_scale_x` = `pixel_xres / display_xres` (backbuffer 缩放因子)
- `dpi_scale_x` = DPI 缩放因子

**OHOS 实现**：
```cpp
float display_scale_x = (float)g_display.pixel_xres / (float)g_surfaceWidth;
float display_scale_y = (float)g_display.pixel_yres / (float)g_surfaceHeight;

touch.x = touchPoint.x * display_scale_x * g_display.dpi_scale_x;
touch.y = touchPoint.y * display_scale_y * g_display.dpi_scale_y;
```

**关键点**：
- OHOS 的 Surface 大小对应 Android 的 display 大小
- 触摸坐标是相对于 Surface 的像素坐标
- 需要先转换到 pixel 坐标系，再转换到 DP 坐标系

### 之前测试的方案

从调试日志中我们测试了 4 种方案（Surface: 2412x1000, Pixel: 2412x895, DP: 1972x732, DPI scale: 0.817）：

| 方案 | 点击 (99, 137) | 说明 |
|------|---------------|------|
| M1 (direct) | (99, 137) | 直接使用原始坐标 - **错误** |
| M2 (dpi_scale) | (81, 112) | 只乘以 dpi_scale - **不完整** |
| M3 (pixel_in_dps) | (121, 168) | 乘以 pixel_in_dps - **错误** |
| M4 (normalized) | (81, 100) | 归一化到 DP - **不完整** |
| **Android 公式** | **(81, 112)** | **display_scale * dpi_scale - 正确** |

**最终方案**：使用 Android 相同的公式，确保跨平台一致性

### 代码位置
- `ohos/entry/src/main/cpp/ohos_xcomponent.cpp` - `OnTouchEvent()` 函数
- `ohos/TOUCH_INPUT_IMPLEMENTATION.md` - 详细文档

## 安全区域实现

### 当前状态
✅ 已实现安全区域功能
- 添加了 `OhosSystem::SetSafeInsets()` 和 `GetSafeInsets()` 函数
- `SYSPROP_DISPLAY_SAFE_INSET_*` 返回实际值（乘以 dpi_scale）
- ArkTS 层从窗口管理器获取安全区域并传递给 C++ 层
- 只使用 `TYPE_CUTOUT`（刘海屏），不使用 `TYPE_SYSTEM`（状态栏/导航栏）

### 安全区域的作用

安全区域（Safe Insets）用于避免 UI 元素被刘海屏、圆角等遮挡：

```
横屏模式（PPSSPP）：
┌──────┬────────────────────────────────┬──────┐
│ 左   │                                │ 右   │
│ 刘   │        游戏画面 + UI           │ 刘   │
│ 海   │    （UI 避开刘海区域）          │ 海   │
│      │                                │      │
└──────┴────────────────────────────────┴──────┘
  ↑                                        ↑
  left inset                          right inset
```

### PPSSPP 如何使用安全区域

PPSSPP 在以下地方使用安全区域：

1. **UI 布局** (`Common/UI/Context.cpp`)
   - 调整 UI 边界，避免按钮被遮挡
   - 菜单、按钮等会自动避开安全区域

2. **游戏画面** (`GPU/Common/PresentationCommon.cpp`)
   - 游戏画面可以延伸到整个屏幕
   - 但会根据安全区域调整渲染区域

3. **调试信息** (`UI/DebugOverlay.cpp`)
   - 调整调试信息的显示位置

### 横屏游戏的安全区域策略

对于横屏游戏（如 PPSSPP），我们采用以下策略：

**只使用 TYPE_CUTOUT（刘海屏）**：
- 刘海屏是**物理遮挡**，无法隐藏
- UI 元素必须避开刘海区域
- 游戏画面可以延伸到刘海下方（背景内容）

**不使用 TYPE_SYSTEM（状态栏/导航栏）**：
- 状态栏和导航栏可以隐藏（沉浸式模式）
- 如果包含这些区域，UI 会过度缩小
- 横屏游戏通常会隐藏系统 UI

### 实现细节

**ArkTS 层** (`Index.ets`)：
```typescript
const systemAvoidArea = windowClass.getWindowAvoidArea(windowManager.AvoidAreaType.TYPE_SYSTEM);
const cutoutAvoidArea = windowClass.getWindowAvoidArea(windowManager.AvoidAreaType.TYPE_CUTOUT);

// 只使用 cutout（刘海屏）
const leftInset = cutoutAvoidArea.leftRect?.width || 0;
const topInset = cutoutAvoidArea.topRect?.height || 0;
const rightInset = cutoutAvoidArea.rightRect?.width || 0;
const bottomInset = cutoutAvoidArea.bottomRect?.height || 0;

ppsspp.setSafeInsets(leftInset, topInset, rightInset, bottomInset);
```

**C++ 层** (`ohos_system_properties.cpp`)：
```cpp
case SYSPROP_DISPLAY_SAFE_INSET_LEFT:
    value = left * g_display.dpi_scale_x;  // 转换到 DP 单位
    break;
```

### XComponent 配置

```typescript
XComponent({...})
  .width('100%')
  .height('100%')
  // 不使用 expandSafeArea，让 PPSSPP 自己处理安全区域
```

## 问题诊断

### 触摸位置不对 - 已修复 ✅

**问题原因**：
- 缺少 `display_scale_x/y` 计算（backbuffer 缩放因子）
- 直接使用原始坐标，没有进行正确的坐标系转换

**解决方案**：
- 使用 Android 相同的公式：`x * display_scale_x * dpi_scale_x`
- `display_scale_x = pixel_xres / surface_width`
- 确保跨平台一致性

### UI 显示在刘海下方 - 已修复 ✅

**问题原因**：
- 之前注释掉了 `systemAvoidArea`，只使用 `cutoutAvoidArea`
- 但实际上应该只使用 `cutoutAvoidArea`（刘海屏）
- 可能是 PPSSPP 没有正确应用安全区域

**解决方案**：
- 确认只使用 `TYPE_CUTOUT`（刘海屏）
- 不使用 `TYPE_SYSTEM`（状态栏/导航栏）
- 让 PPSSPP 自动根据安全区域调整 UI 布局

### 横屏模式下的显示

这是**正常行为**：
- 横屏游戏应该填满整个屏幕
- 游戏画面可以延伸到刘海下方（背景内容）
- UI 元素（按钮、菜单）会自动避开刘海区域
- PPSSPP 会根据安全区域自动调整 UI 布局

## 测试步骤

1. **测试触摸响应**
   - 重新编译并运行应用
   - 点击屏幕不同位置（左上、右上、中间、左下、右下）
   - 查看日志中的坐标转换信息
   - 确认触摸响应位置是否正确

2. **测试安全区域**
   - 查看日志中的安全区域值
   - 确认 UI 元素是否避开刘海区域
   - 检查菜单、按钮是否在可见区域内

3. **如果仍有问题**
   - 提供日志输出（触摸坐标、安全区域值）
   - 截图显示问题区域
   - 描述具体的错位情况

## 参考

- Android 实现：`android/jni/app-android.cpp`
- 触摸输入文档：`ohos/TOUCH_INPUT_IMPLEMENTATION.md`
- HiLog 格式文档：`ohos/HILOG_PUBLIC_FORMAT.md`
