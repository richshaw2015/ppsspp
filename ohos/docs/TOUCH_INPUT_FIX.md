# 触摸输入坐标转换修复

## 问题描述

用户报告触摸输入的响应位置不正确，点击位置与实际响应位置有明显错位。

## 根本原因

OHOS 实现缺少了 Android 中的 `display_scale_x/y` 概念（backbuffer 缩放因子）。

### Android 实现

```cpp
// android/jni/app-android.cpp
touch.x = x * display_scale_x * g_display.dpi_scale_x;
touch.y = y * display_scale_y * g_display.dpi_scale_y;
```

其中：
- `display_scale_x` = `pixel_xres / display_xres` (backbuffer 到 display 的缩放比例)
- `dpi_scale_x` = DPI 缩放因子（像素到 DP 的转换）

### OHOS 之前的实现

```cpp
// 错误：直接使用原始坐标
touch.x = touchPoint.x;
touch.y = touchPoint.y;
```

这导致触摸坐标没有经过正确的坐标系转换。

## 解决方案

### 1. 计算 display_scale

在 OHOS 中：
- Surface 大小对应 Android 的 display 大小
- Pixel 大小是实际的渲染分辨率
- 因此：`display_scale_x = pixel_xres / surface_width`

### 2. 应用 Android 公式

```cpp
float display_scale_x = (float)g_display.pixel_xres / (float)g_surfaceWidth;
float display_scale_y = (float)g_display.pixel_yres / (float)g_surfaceHeight;

touch.x = touchPoint.x * display_scale_x * g_display.dpi_scale_x;
touch.y = touchPoint.y * display_scale_y * g_display.dpi_scale_y;
```

### 3. 坐标转换流程

```
触摸坐标（Surface 像素）
    ↓ × display_scale_x
Pixel 坐标（渲染分辨率）
    ↓ × dpi_scale_x
DP 坐标（设备独立像素）
    ↓
PPSSPP UI 坐标系
```

## 测试数据示例

假设：
- Surface: 2412 × 1000
- Pixel: 2412 × 895
- DP: 1972 × 732
- DPI scale: 0.817

计算：
- `display_scale_x = 2412 / 2412 = 1.0`
- `display_scale_y = 895 / 1000 = 0.895`

触摸点 (99, 137)：
- `x = 99 × 1.0 × 0.817 = 80.9`
- `y = 137 × 0.895 × 0.817 = 100.2`

这与之前测试的 M2 方案结果一致，但现在我们理解了为什么需要这样转换。

## 代码变更

### 文件：`ohos/entry/src/main/cpp/ohos_xcomponent.cpp`

**修改前**：
```cpp
touch.x = touchPoint.x;
touch.y = touchPoint.y;
```

**修改后**：
```cpp
float display_scale_x = (float)g_display.pixel_xres / (float)g_surfaceWidth;
float display_scale_y = (float)g_display.pixel_yres / (float)g_surfaceHeight;

touch.x = touchPoint.x * display_scale_x * g_display.dpi_scale_x;
touch.y = touchPoint.y * display_scale_y * g_display.dpi_scale_y;
```

## 调试日志

添加了详细的调试日志，输出：
- 原始触摸坐标
- display_scale 值
- dpi_scale 值
- 最终转换后的 DP 坐标

```cpp
OHOS_LOGI(XCOMP_TAG, "Touch DOWN: raw=(%.1f,%.1f) scale=(%.3f,%.3f) dpi=(%.3f,%.3f) -> dp=(%.1f,%.1f)", 
          touchPoint.x, touchPoint.y,
          display_scale_x, display_scale_y,
          g_display.dpi_scale_x, g_display.dpi_scale_y,
          touch.x, touch.y);
```

## 跨平台一致性

现在 OHOS 实现与 Android 实现使用相同的坐标转换公式，确保：
- 触摸行为一致
- 代码逻辑清晰
- 易于维护和调试

## 相关文件

- `ohos/entry/src/main/cpp/ohos_xcomponent.cpp` - 触摸输入实现
- `ohos/TOUCH_AND_SAFE_AREA_STATUS.md` - 状态文档
- `android/jni/app-android.cpp` - Android 参考实现
- `Common/System/Display.h` - 显示属性定义

## 测试建议

1. 重新编译并运行应用
2. 点击屏幕不同位置
3. 查看日志中的坐标转换信息
4. 确认触摸响应位置是否正确
5. 测试多点触控是否正常

## 注意事项

- 确保 `g_surfaceWidth` 和 `g_surfaceHeight` 已正确初始化
- 确保 `g_display.pixel_xres/yres` 和 `dpi_scale_x/y` 已正确设置
- 如果 Surface 大小改变，`display_scale` 会自动重新计算
