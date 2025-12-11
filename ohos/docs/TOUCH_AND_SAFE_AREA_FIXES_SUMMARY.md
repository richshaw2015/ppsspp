# 触摸输入和安全区域修复总结

## 修复内容

本次修复解决了两个主要问题：
1. **触摸输入坐标转换错误** - 点击位置与响应位置不匹配
2. **安全区域策略优化** - UI 显示在刘海屏下方

---

## 1. 触摸输入坐标转换修复

### 问题
触摸输入的响应位置不正确，点击位置与实际响应位置有明显错位。

### 根本原因
OHOS 实现缺少了 Android 中的 `display_scale_x/y` 概念（backbuffer 缩放因子），直接使用原始坐标导致坐标系转换错误。

### 解决方案
采用与 Android 相同的坐标转换公式：

```cpp
// 计算 display_scale（backbuffer 缩放因子）
float display_scale_x = (float)g_display.pixel_xres / (float)g_surfaceWidth;
float display_scale_y = (float)g_display.pixel_yres / (float)g_surfaceHeight;

// 应用 Android 公式
touch.x = touchPoint.x * display_scale_x * g_display.dpi_scale_x;
touch.y = touchPoint.y * display_scale_y * g_display.dpi_scale_y;
```

### 坐标转换流程
```
触摸坐标（Surface 像素）
    ↓ × display_scale_x
Pixel 坐标（渲染分辨率）
    ↓ × dpi_scale_x
DP 坐标（设备独立像素）
    ↓
PPSSPP UI 坐标系
```

### 修改文件
- `ohos/entry/src/main/cpp/ohos_xcomponent.cpp` - `OnTouchEvent()` 函数

---

## 2. 安全区域策略优化

### 问题
UI 元素显示在刘海屏下方，被物理遮挡。

### 根本原因
安全区域策略不明确，可能同时使用了 `TYPE_SYSTEM` 和 `TYPE_CUTOUT`。

### 解决方案
明确横屏游戏的安全区域策略：

**只使用 TYPE_CUTOUT（刘海屏）**：
- 刘海屏是物理遮挡，无法隐藏
- UI 元素必须避开刘海区域
- 游戏画面可以延伸到刘海下方

**不使用 TYPE_SYSTEM（状态栏/导航栏）**：
- 状态栏和导航栏可以隐藏（沉浸式模式）
- 横屏游戏通常会隐藏系统 UI
- 如果包含这些区域，UI 会过度缩小

### 实现代码
```typescript
// 只使用 cutout（刘海屏）
const leftInset = cutoutAvoidArea.leftRect?.width || 0;
const topInset = cutoutAvoidArea.topRect?.height || 0;
const rightInset = cutoutAvoidArea.rightRect?.width || 0;
const bottomInset = cutoutAvoidArea.bottomRect?.height || 0;

ppsspp.setSafeInsets(leftInset, topInset, rightInset, bottomInset);
```

### 修改文件
- `ohos/entry/src/main/ets/pages/Index.ets` - `aboutToAppear()` 函数

---

## 修改的文件列表

### 代码文件
1. **ohos/entry/src/main/cpp/ohos_xcomponent.cpp**
   - 修复触摸坐标转换公式
   - 添加详细的调试日志

2. **ohos/entry/src/main/ets/pages/Index.ets**
   - 优化安全区域获取策略
   - 只使用 TYPE_CUTOUT
   - 改进日志输出
   - 移除未使用的 window 导入

### 文档文件
3. **ohos/TOUCH_AND_SAFE_AREA_STATUS.md**
   - 更新触摸输入实现说明
   - 更新安全区域策略说明
   - 更新问题诊断部分

4. **ohos/TOUCH_INPUT_FIX.md** (新建)
   - 详细说明触摸输入修复过程
   - 包含代码示例和测试数据

5. **ohos/TOUCH_AND_SAFE_AREA_FIXES_SUMMARY.md** (本文件)
   - 修复内容总结

---

## 技术细节

### 触摸坐标转换

**关键概念**：
- **Surface 坐标**：XComponent 表面的像素坐标（触摸事件的原始坐标）
- **Pixel 坐标**：实际渲染分辨率（backbuffer）
- **DP 坐标**：设备独立像素（PPSSPP UI 使用的坐标系）

**转换公式**：
```
DP = Surface × (Pixel / Surface) × (DP / Pixel)
   = Surface × display_scale × dpi_scale
```

**示例**（Surface: 2412×1000, Pixel: 2412×895, DP: 1972×732）：
- `display_scale_x = 2412 / 2412 = 1.0`
- `display_scale_y = 895 / 1000 = 0.895`
- `dpi_scale_x = 1972 / 2412 = 0.817`
- `dpi_scale_y = 732 / 895 = 0.818`

触摸点 (99, 137) 转换为：
- `x = 99 × 1.0 × 0.817 = 80.9`
- `y = 137 × 0.895 × 0.818 = 100.2`

### 安全区域处理

**OHOS 避让区域类型**：
- `TYPE_SYSTEM`：系统 UI（状态栏、导航栏）- 可隐藏
- `TYPE_CUTOUT`：刘海屏、圆角 - 物理遮挡，不可隐藏

**横屏游戏策略**：
```
┌──────┬────────────────────────────────┬──────┐
│ 左   │                                │ 右   │
│ 刘   │        游戏画面 + UI           │ 刘   │
│ 海   │    （UI 避开刘海区域）          │ 海   │
│      │                                │      │
└──────┴────────────────────────────────┴──────┘
  ↑                                        ↑
  left inset                          right inset
```

**PPSSPP 自动处理**：
- UI 布局会根据安全区域自动调整
- 游戏画面可以延伸到整个屏幕
- 按钮、菜单等会避开刘海区域

---

## 调试日志

### 触摸输入日志
```
Touch DOWN: raw=(99.0,137.0) scale=(1.000,0.895) dpi=(0.817,0.818) -> dp=(80.9,100.2)
```

### 安全区域日志
```
[PPSSPP] systemAvoidArea - left: 0, top: 48, right: 0, bottom: 0
[PPSSPP] cutoutAvoidArea - left: 80, top: 0, right: 80, bottom: 0
[PPSSPP] Final safe insets (cutout only): left=80, top=0, right=80, bottom=0
```

---

## 测试步骤

### 1. 编译和运行
```bash
cd ohos
./rebuild.sh
```

### 2. 测试触摸输入
- 点击屏幕不同位置（左上、右上、中间、左下、右下）
- 查看日志中的坐标转换信息
- 确认触摸响应位置是否正确

### 3. 测试安全区域
- 查看日志中的安全区域值
- 确认 UI 元素是否避开刘海区域
- 检查菜单、按钮是否在可见区域内

### 4. 测试多点触控
- 使用两个手指同时触摸
- 确认多点触控是否正常工作

---

## 预期结果

### 触摸输入
✅ 点击位置与响应位置准确匹配
✅ 多点触控正常工作
✅ 触摸响应流畅

### 安全区域
✅ UI 元素不被刘海遮挡
✅ 游戏画面填满整个屏幕
✅ 按钮、菜单在可见区域内

---

## 跨平台一致性

现在 OHOS 实现与 Android 实现保持一致：
- 使用相同的坐标转换公式
- 使用相同的安全区域处理逻辑
- 代码结构清晰，易于维护

---

## 相关文档

- `ohos/TOUCH_INPUT_FIX.md` - 触摸输入修复详情
- `ohos/TOUCH_AND_SAFE_AREA_STATUS.md` - 实现状态文档
- `ohos/HILOG_PUBLIC_FORMAT.md` - HiLog 日志格式说明
- `android/jni/app-android.cpp` - Android 参考实现
- `Common/System/Display.h` - 显示属性定义

---

## 注意事项

1. **触摸坐标转换**
   - 确保 `g_surfaceWidth/Height` 已正确初始化
   - 确保 `g_display.pixel_xres/yres` 已正确设置
   - 确保 `g_display.dpi_scale_x/y` 已正确计算

2. **安全区域**
   - 只在横屏模式下使用 TYPE_CUTOUT
   - 不要使用 TYPE_SYSTEM（会导致 UI 过度缩小）
   - 让 PPSSPP 自动处理 UI 布局

3. **调试**
   - 使用 `%{public}` 格式打印日志值
   - 查看触摸坐标转换的中间值
   - 查看安全区域的实际值

---

## 后续工作

如果仍有问题：
1. 提供完整的日志输出
2. 提供截图显示问题区域
3. 描述具体的错位情况
4. 提供设备型号和屏幕分辨率信息
