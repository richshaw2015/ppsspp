# 圆角屏幕安全区域处理

## 问题

鸿蒙系统的避让区域 API 存在局限性：
- `TYPE_CUTOUT` 只报告物理刘海（摄像头、传感器）
- `TYPE_SYSTEM` 只报告系统 UI（状态栏、导航栏）
- **没有专门的 API 报告屏幕圆角**

实际情况：
- 左侧刘海：71×72 像素 ✅ 可以检测到
- 右侧刘海：0×0 像素 ❌ 检测不到（但实际有圆角）
- 系统避让区域：全部为 0 ❌ 横屏模式下系统 UI 隐藏

## 解决方案

### 策略：添加最小安全边距

为了避免 UI 元素被圆角遮挡，我们添加一个**最小安全边距**：

```typescript
// 圆角屏幕的最小安全边距（像素）
const MIN_CORNER_INSET = 40;

// 获取刘海区域
const cutoutLeft = cutoutAvoidArea.leftRect?.width || 0;
const cutoutRight = cutoutAvoidArea.rightRect?.width || 0;

// 合并刘海区域和圆角边距（取最大值）
const leftInset = Math.max(cutoutLeft, MIN_CORNER_INSET);
const rightInset = Math.max(cutoutRight, MIN_CORNER_INSET);
```

### 为什么是 40 像素？

根据常见设备的圆角半径：
- **小圆角**：20-30 像素（较老的设备）
- **中等圆角**：30-40 像素（大多数现代设备）
- **大圆角**：40-50 像素（旗舰设备）

选择 40 像素作为默认值：
- ✅ 覆盖大多数设备的圆角
- ✅ 不会过度缩小游戏区域
- ✅ 提供足够的视觉缓冲

### 实际效果（对称策略）

**检测到的值**：
```
cutoutLeft = 71   ← 左侧刘海
cutoutRight = 0   ← 右侧圆角（检测不到）
MIN_CORNER_INSET = 40
```

**计算对称边距**：
```
horizontalInset = Math.max(71, 0, 40) = 71
leftInset = 71   ← 使用最大值
rightInset = 71  ← 左右对称
```

**结果**：
- 左侧：71 像素安全边距（刘海）
- 右侧：71 像素安全边距（对称）
- 游戏画面左右对称，视觉更平衡 ✅
- UI 元素不会被遮挡 ✅

## 可视化说明

### 对称策略（推荐）

```
横屏模式 - 左右对称：
┌──────────┬──────────────────────────┬──────────┐
│ 左侧刘海 │                          │ 右侧圆角 │
│ 71px     │      游戏画面 + UI       │ 71px     │
│          │                          │          │
│  ┌─┐     │                          │      ╭─┐ │
│  │●│     │   UI 元素在安全区域内    │      ╰─╯ │
│  └─┘     │                          │          │
└──────────┴──────────────────────────┴──────────┘
   ↑                                      ↑
   刘海 71px                              对称 71px
   
优点：
✅ 游戏画面左右对称，视觉平衡
✅ UI 布局更美观
✅ 确保两侧都不被遮挡
```

### 非对称策略（旧方案）

```
横屏模式 - 左右不对称：
┌──────────┬──────────────────────────┬────────┐
│ 左侧刘海 │                          │ 右侧   │
│ 71px     │      游戏画面 + UI       │ 圆角   │
│          │                          │ 40px   │
│  ┌─┐     │                          │    ╭─┐ │
│  │●│     │                          │    ╰─╯ │
│  └─┘     │                          │        │
└──────────┴──────────────────────────┴────────┘
   ↑                                      ↑
   刘海 71px                              圆角 40px
   
缺点：
❌ 游戏画面不对称，视觉不平衡
❌ UI 布局可能偏向一侧
```

## 代码实现

### Index.ets（对称策略）

```typescript
// 圆角屏幕的最小安全边距（像素）
const MIN_CORNER_INSET = 40;

// 获取刘海区域
const cutoutLeft = cutoutAvoidArea.leftRect?.width || 0;
const cutoutTop = cutoutAvoidArea.topRect?.height || 0;
const cutoutRight = cutoutAvoidArea.rightRect?.width || 0;
const cutoutBottom = cutoutAvoidArea.bottomRect?.height || 0;

// 横屏模式：左右对称（取左右两侧的最大值）
const horizontalInset = Math.max(cutoutLeft, cutoutRight, MIN_CORNER_INSET);
const verticalInset = Math.max(cutoutTop, cutoutBottom, MIN_CORNER_INSET);

const leftInset = horizontalInset;
const topInset = verticalInset;
const rightInset = horizontalInset;
const bottomInset = verticalInset;

console.info(`[PPSSPP] Cutout insets: left=${cutoutLeft}, top=${cutoutTop}, right=${cutoutRight}, bottom=${cutoutBottom}`);
console.info(`[PPSSPP] Final safe insets (symmetric): left=${leftInset}, top=${topInset}, right=${rightInset}, bottom=${bottomInset}`);

ppsspp.setSafeInsets(leftInset, topInset, rightInset, bottomInset);
```

**关键点**：
- `horizontalInset`：取左右刘海和最小边距的最大值
- 左右使用相同的 `horizontalInset`，确保对称
- 上下也使用相同的 `verticalInset`，确保对称

## 日志示例

### 有刘海的设备（对称策略）
```
[PPSSPP] cutoutAvoidArea - left: 71X72, right: 0X0
[PPSSPP] Cutout insets: left=71, top=0, right=0, bottom=0
[PPSSPP] Final safe insets (symmetric): left=71, top=40, right=71, bottom=40
                                              ↑              ↑
                                           左右对称 71px
```

### 无刘海的设备（只有圆角）
```
[PPSSPP] cutoutAvoidArea - left: 0X0, right: 0X0
[PPSSPP] Cutout insets: left=0, top=0, right=0, bottom=0
[PPSSPP] Final safe insets (symmetric): left=40, top=40, right=40, bottom=40
                                              ↑              ↑
                                           左右对称 40px
```

### 两侧都有刘海的设备
```
[PPSSPP] cutoutAvoidArea - left: 71X72, right: 71X72
[PPSSPP] Cutout insets: left=71, top=0, right=71, bottom=0
[PPSSPP] Final safe insets (symmetric): left=71, top=40, right=71, bottom=40
                                              ↑              ↑
                                           本来就对称
```

## 调整建议

如果 40 像素不够或太多，可以调整 `MIN_CORNER_INSET` 值：

### 减小边距（更大的游戏区域）
```typescript
const MIN_CORNER_INSET = 30;  // 适合小圆角设备
```

### 增大边距（更保守的安全区域）
```typescript
const MIN_CORNER_INSET = 50;  // 适合大圆角设备
```

### 根据设备动态调整（高级）
```typescript
// 根据屏幕尺寸动态计算
const screenWidth = windowClass.getWindowProperties().windowRect.width;
const MIN_CORNER_INSET = Math.floor(screenWidth * 0.02);  // 屏幕宽度的 2%
```

## 其他平台的处理

### Android
Android 也有类似问题，通常使用：
- `WindowInsets.getDisplayCutout()` - 刘海
- `WindowInsets.getSystemWindowInsets()` - 系统 UI + 圆角
- 或者使用固定的安全边距

### iOS
iOS 提供 `safeAreaInsets`，自动包含：
- 刘海（Face ID 区域）
- 圆角
- Home Indicator

鸿蒙未来可能会提供类似的完整 API。

## 注意事项

1. **最小边距是经验值**
   - 不同设备的圆角半径不同
   - 40 像素是一个合理的折中值
   - 可以根据实际测试调整

2. **不影响游戏画面**
   - 游戏画面仍然填满整个屏幕
   - 只有 UI 元素（按钮、菜单）会避开边缘
   - PPSSPP 会自动处理布局

3. **用户可能看到圆角**
   - 游戏背景可能延伸到圆角下方
   - 这是正常的，不影响游戏体验
   - 重要的 UI 元素不会被遮挡

## 未来改进

如果鸿蒙提供了圆角 API（如 `TYPE_ROUNDED_CORNER`），可以直接使用：

```typescript
// 未来可能的 API
const cornerAvoidArea = windowClass.getWindowAvoidArea(
  windowManager.AvoidAreaType.TYPE_ROUNDED_CORNER
);

const leftInset = Math.max(
  cutoutAvoidArea.leftRect?.width || 0,
  cornerAvoidArea.leftRect?.width || 0
);
```

在此之前，使用最小边距是最可靠的解决方案。

## 相关文档

- `ohos/entry/src/main/ets/pages/Index.ets` - 实现代码
- `ohos/TOUCH_AND_SAFE_AREA_STATUS.md` - 安全区域总体说明
- `ohos/TOUCH_AND_SAFE_AREA_FIXES_SUMMARY.md` - 修复总结
