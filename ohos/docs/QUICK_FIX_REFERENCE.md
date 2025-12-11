# 快速修复参考

## 本次修复的问题

### 1. 触摸位置不对 ✅
**症状**：点击位置与响应位置错位

**修复**：使用 Android 相同的坐标转换公式
```cpp
float display_scale_x = (float)g_display.pixel_xres / (float)g_surfaceWidth;
float display_scale_y = (float)g_display.pixel_yres / (float)g_surfaceHeight;
touch.x = touchPoint.x * display_scale_x * g_display.dpi_scale_x;
touch.y = touchPoint.y * display_scale_y * g_display.dpi_scale_y;
```

**文件**：`ohos/entry/src/main/cpp/ohos_xcomponent.cpp`

---

### 2. UI 显示在刘海下方 ✅
**症状**：UI 元素被刘海屏遮挡

**修复**：只使用 TYPE_CUTOUT（刘海屏），不使用 TYPE_SYSTEM（状态栏）
```typescript
const leftInset = cutoutAvoidArea.leftRect?.width || 0;
const topInset = cutoutAvoidArea.topRect?.height || 0;
const rightInset = cutoutAvoidArea.rightRect?.width || 0;
const bottomInset = cutoutAvoidArea.bottomRect?.height || 0;
ppsspp.setSafeInsets(leftInset, topInset, rightInset, bottomInset);
```

**文件**：`ohos/entry/src/main/ets/pages/Index.ets`

---

## 如何测试

### 编译运行
```bash
cd ohos
./rebuild.sh
```

### 查看日志
```bash
# 触摸输入日志
hdc shell hilog | grep "Touch DOWN"

# 安全区域日志
hdc shell hilog | grep "safe insets"
```

### 预期日志输出
```
Touch DOWN: raw=(99.0,137.0) scale=(1.000,0.895) dpi=(0.817,0.818) -> dp=(80.9,100.2)
Final safe insets (cutout only): left=80, top=0, right=80, bottom=0
```

---

## 关键概念

### 坐标转换
```
Surface 坐标 → Pixel 坐标 → DP 坐标
    (×display_scale)  (×dpi_scale)
```

### 安全区域
- **TYPE_CUTOUT**：刘海屏（物理遮挡）← 使用这个
- **TYPE_SYSTEM**：状态栏/导航栏（可隐藏）← 不使用

---

## 相关文档

- `TOUCH_AND_SAFE_AREA_FIXES_SUMMARY.md` - 完整修复说明
- `TOUCH_INPUT_FIX.md` - 触摸输入详细说明
- `TOUCH_AND_SAFE_AREA_STATUS.md` - 实现状态
- `HILOG_PUBLIC_FORMAT.md` - 日志格式说明
