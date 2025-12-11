# 对称安全区域策略

## 为什么要左右对称？

### 视觉效果对比

**非对称（旧方案）**：
```
┌──────────┬──────────────────────────┬────────┐
│          │                          │        │
│  71px    │      游戏画面            │  40px  │
│          │                          │        │
└──────────┴──────────────────────────┴────────┘
           ↑ 画面偏向右侧 ↑
```
- ❌ 游戏画面不居中
- ❌ UI 布局可能偏向一侧
- ❌ 视觉不平衡

**对称（新方案）**：
```
┌──────────┬──────────────────────────┬──────────┐
│          │                          │          │
│  71px    │      游戏画面            │  71px    │
│          │                          │          │
└──────────┴──────────────────────────┴──────────┘
           ↑ 画面居中 ↑
```
- ✅ 游戏画面完美居中
- ✅ UI 布局对称美观
- ✅ 视觉平衡舒适

## 实现原理

### 代码逻辑

```typescript
// 获取左右两侧的刘海宽度
const cutoutLeft = 71;   // 左侧刘海
const cutoutRight = 0;   // 右侧圆角（检测不到）

// 取左右两侧的最大值，加上最小圆角边距
const horizontalInset = Math.max(cutoutLeft, cutoutRight, MIN_CORNER_INSET);
// horizontalInset = Math.max(71, 0, 40) = 71

// 左右都使用相同的值
const leftInset = horizontalInset;   // 71
const rightInset = horizontalInset;  // 71
```

### 为什么这样做？

1. **刘海通常只在一侧**
   - 大多数设备：左侧有刘海，右侧只有圆角
   - 如果不对称，游戏画面会偏向一侧

2. **圆角无法检测**
   - 鸿蒙 API 检测不到圆角
   - 但圆角实际存在，需要预留空间

3. **对称更美观**
   - 游戏画面居中显示
   - UI 元素对称分布
   - 符合用户视觉习惯

## 实际效果

### 你的设备（左侧刘海 71px）

**对称策略**：
```
左侧边距：71px（刘海）
右侧边距：71px（对称）
游戏画面：居中显示
```

**非对称策略**：
```
左侧边距：71px（刘海）
右侧边距：40px（圆角）
游戏画面：偏右 15.5px
```

差异：游戏画面偏移 `(71-40)/2 = 15.5` 像素

### 其他设备场景

| 设备类型 | 左侧刘海 | 右侧刘海 | 对称结果 | 说明 |
|---------|---------|---------|---------|------|
| 单侧刘海 | 71px | 0px | 71px / 71px | 居中显示 ✅ |
| 双侧刘海 | 71px | 71px | 71px / 71px | 本来就对称 ✅ |
| 无刘海 | 0px | 0px | 40px / 40px | 圆角保护 ✅ |
| 不对称刘海 | 71px | 50px | 71px / 71px | 取最大值 ✅ |

## 对比其他平台

### iOS
iOS 的 `safeAreaInsets` 通常是对称的：
```swift
// iPhone X 系列
safeAreaInsets.left = 44
safeAreaInsets.right = 44  // 对称
```

### Android
Android 游戏通常也采用对称策略：
```java
// 取左右两侧的最大值
int horizontalInset = Math.max(leftCutout, rightCutout);
```

### PPSSPP 其他平台
PPSSPP 在其他平台上也倾向于对称布局，确保游戏画面居中。

## 用户体验

### 对称的优势

1. **视觉舒适**
   - 游戏画面居中，符合视觉习惯
   - 左右平衡，不会感觉"歪"

2. **UI 布局**
   - 按钮、菜单对称分布
   - 更容易找到控制元素

3. **沉浸感**
   - 画面居中，更有沉浸感
   - 不会因为偏移而分散注意力

### 可能的缺点

1. **稍微减少游戏区域**
   - 右侧从 40px 增加到 71px
   - 损失约 31px 的宽度
   - 但换来更好的视觉体验

2. **对于无刘海设备**
   - 如果两侧都没有刘海，仍然是对称的
   - 使用最小边距 40px，影响很小

## 如何调整？

### 如果觉得边距太大

可以减小最小边距：
```typescript
const MIN_CORNER_INSET = 30;  // 从 40 减到 30
```

### 如果想要非对称

可以改回独立计算：
```typescript
const leftInset = Math.max(cutoutLeft, MIN_CORNER_INSET);
const rightInset = Math.max(cutoutRight, MIN_CORNER_INSET);
```

### 如果想要更激进的对称

可以同时对称上下：
```typescript
const maxInset = Math.max(cutoutLeft, cutoutRight, cutoutTop, cutoutBottom, MIN_CORNER_INSET);
const leftInset = maxInset;
const topInset = maxInset;
const rightInset = maxInset;
const bottomInset = maxInset;
```

## 测试建议

重新编译后，观察：
1. 游戏画面是否居中
2. UI 元素是否对称分布
3. 是否有元素被遮挡
4. 整体视觉是否舒适

查看日志确认：
```
[PPSSPP] Final safe insets (symmetric): left=71, top=40, right=71, bottom=40
```

左右应该相同（71 = 71），上下应该相同（40 = 40）。

## 总结

对称策略的核心思想：
- **横屏游戏应该居中显示**
- **左右对称比精确计算更重要**
- **视觉平衡优于最大化游戏区域**

这是大多数游戏采用的策略，也是用户期望的体验。
