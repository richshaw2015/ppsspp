# OHOS 触摸输入实现

## 概述

PPSSPP OHOS 平台的触摸输入通过 XComponent 的触摸事件回调实现，支持多点触控和手势识别。

## 实现架构

```
┌─────────────────────────────────────────────────────────────┐
│ 1. OHOS 系统触摸事件                                         │
│    用户触摸屏幕 → XComponent 捕获事件                        │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 2. XComponent 回调：DispatchTouchEvent                       │
│    OH_NativeXComponent_GetTouchEvent()                      │
│    - 获取触摸点数量                                          │
│    - 获取每个触摸点的坐标和 ID                                │
│    - 获取触摸动作类型                                        │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 3. OnTouchEvent 处理                                        │
│    - 坐标转换（DPI 缩放）                                    │
│    - 动作映射（DOWN/MOVE/UP/CANCEL）                        │
│    - 创建 TouchInput 结构                                    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 4. PPSSPP 输入系统                                          │
│    NativeTouch(TouchInput) → 输入处理器                     │
│    - UI 交互（按钮点击、滑动）                               │
│    - 游戏控制（虚拟摇杆、按键）                              │
│    - 手势识别（缩放、旋转）                                  │
└─────────────────────────────────────────────────────────────┘
```

## 核心代码

### 触摸事件处理函数

```cpp
void OnTouchEvent(OH_NativeXComponent* component, void* window) {
    // 1. 获取触摸事件
    OH_NativeXComponent_TouchEvent touchEvent;
    OH_NativeXComponent_GetTouchEvent(component, window, &touchEvent);
    
    // 2. 处理每个触摸点
    for (uint32_t i = 0; i < touchEvent.numPoints; i++) {
        OH_NativeXComponent_TouchPoint touchPoint = touchEvent.touchPoints[i];
        
        // 3. 创建 PPSSPP 触摸输入
        TouchInput touch{};
        touch.id = touchPoint.id;
        // 从屏幕像素转换到 dp
        touch.x = touchPoint.x * g_display.pixel_in_dps_x;
        touch.y = touchPoint.y * g_display.pixel_in_dps_y;
        
        // 4. 映射动作类型
        switch (touchEvent.type) {
            case OH_NATIVEXCOMPONENT_DOWN:  touch.flags = TOUCH_DOWN;   break;
            case OH_NATIVEXCOMPONENT_UP:    touch.flags = TOUCH_UP;     break;
            case OH_NATIVEXCOMPONENT_MOVE:  touch.flags = TOUCH_MOVE;   break;
            case OH_NATIVEXCOMPONENT_CANCEL: touch.flags = TOUCH_CANCEL; break;
        }
        
        // 5. 发送到 PPSSPP
        NativeTouch(touch);
    }
}
```

## 触摸事件类型

### OHOS 触摸动作
- `OH_NATIVEXCOMPONENT_DOWN` - 手指按下
- `OH_NATIVEXCOMPONENT_UP` - 手指抬起
- `OH_NATIVEXCOMPONENT_MOVE` - 手指移动
- `OH_NATIVEXCOMPONENT_CANCEL` - 触摸取消

### PPSSPP 触摸标志
- `TOUCH_DOWN` (1 << 1) - 按下事件
- `TOUCH_UP` (1 << 2) - 抬起事件
- `TOUCH_MOVE` (1 << 0) - 移动事件
- `TOUCH_CANCEL` (1 << 3) - 取消事件

## 坐标转换

### 像素到 DP 转换
```cpp
touch.x = touchPoint.x * g_display.pixel_in_dps_x;
touch.y = touchPoint.y * g_display.pixel_in_dps_y;
```

### 坐标系统
- **OHOS 坐标**：屏幕像素坐标（相对于 XComponent）
- **PPSSPP 坐标**：密度无关像素（dp, density-independent pixels）
- **转换因子**：`pixel_in_dps_x/y` 直接将像素转换为 dp

### 与 Android 的对比
Android 使用两级缩放：
```cpp
// Android 实现
touch.x = x * display_scale_x * g_display.dpi_scale_x;
touch.y = y * display_scale_y * g_display.dpi_scale_y;
```
- `display_scale_x/y`：backbuffer 到 display 的缩放
- `dpi_scale_x/y`：DPI 缩放因子

OHOS 简化为一级缩放：
```cpp
// OHOS 实现
touch.x = touchPoint.x * g_display.pixel_in_dps_x;
touch.y = touchPoint.y * g_display.pixel_in_dps_y;
```
- `pixel_in_dps_x/y`：直接从像素转换到 dp

## 多点触控支持

### 触摸点 ID
- 每个触摸点有唯一的 ID
- ID 在触摸生命周期内保持不变
- 最多支持 `OH_MAX_TOUCH_POINTS_NUMBER` 个触摸点

### 触摸点数组
```cpp
for (uint32_t i = 0; i < touchEvent.numPoints; i++) {
    OH_NativeXComponent_TouchPoint touchPoint = touchEvent.touchPoints[i];
    // 处理每个触摸点
}
```

## 手势识别

PPSSPP 内置的手势识别器会自动处理：
- **单击** - UI 按钮点击
- **长按** - 上下文菜单
- **滑动** - 列表滚动
- **双指缩放** - 屏幕缩放
- **双指旋转** - 屏幕旋转

## 虚拟控制器

### 虚拟按键
PPSSPP 会在屏幕上显示虚拟按键：
- 方向键（D-Pad）
- 动作按钮（○×□△）
- 肩键（L/R）
- 摇杆

### 触摸映射
触摸事件会自动映射到虚拟按键：
1. 检测触摸点是否在按键区域内
2. 按下时发送按键 DOWN 事件
3. 抬起时发送按键 UP 事件

## 调试

### 启用触摸调试日志
在 `ohos_xcomponent.cpp` 中添加：
```cpp
OHOS_LOGD(XCOMP_TAG, "Touch: id=%d, x=%.2f, y=%.2f, flags=%d", 
          touch.id, touch.x, touch.y, touch.flags);
```

### 查看触摸事件
```bash
hdc shell hilog | grep "Touch"
```

## 性能优化

### 1. 事件过滤
```cpp
// 过滤重复的 MOVE 事件
static float lastX[10] = {0};
static float lastY[10] = {0};

if (touch.flags == TOUCH_MOVE) {
    float dx = touch.x - lastX[touch.id];
    float dy = touch.y - lastY[touch.id];
    if (dx*dx + dy*dy < 1.0f) {
        continue;  // 移动距离太小，忽略
    }
}
```

### 2. 批处理
OHOS 已经批处理了同一帧的多个触摸点，无需额外处理。

## 已知限制

1. **触摸延迟**：约 16ms（60fps）
2. **最大触摸点**：取决于设备，通常 10 个
3. **触摸精度**：取决于屏幕分辨率

## 测试

### 基本触摸测试
1. 启动 PPSSPP
2. 点击 UI 按钮
3. 验证按钮响应

### 多点触控测试
1. 使用两个手指同时触摸
2. 验证缩放手势
3. 验证旋转手势

### 虚拟控制器测试
1. 加载游戏
2. 点击虚拟按键
3. 验证游戏响应

## 相关文件

- `ohos/entry/src/main/cpp/ohos_xcomponent.cpp` - 触摸事件处理
- `Common/Input/InputState.h` - TouchInput 结构定义
- `Common/System/NativeApp.h` - NativeTouch 函数声明
- `UI/TouchControlVisibilityScreen.cpp` - 虚拟控制器配置

## 参考

- Android 实现：`android/jni/app-android.cpp`
- OHOS XComponent 文档：[XComponent 开发指南](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/ndk-xcomponent-guidelines-0000001281201106-V3)
