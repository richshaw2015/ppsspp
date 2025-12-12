# OHOS 平台输入系统实现

## 概述

实现了 OHOS 平台的完整输入系统，包括触摸、按键和手柄支持。

## 架构

```
┌─────────────────────────────────────────────────────────────┐
│                      ArkTS 层                                │
├─────────────────────────────────────────────────────────────┤
│  XComponent.onKeyEvent() ──→ ppsspp.sendKeyEvent()          │
│  (手柄事件)              ──→ ppsspp.sendAxisEvent()         │
│                          ──→ ppsspp.sendMultiAxisEvent()    │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      NAPI 层                                 │
├─────────────────────────────────────────────────────────────┤
│  NapiPPSSPP::SendKeyEvent()                                 │
│  NapiPPSSPP::SendAxisEvent()                                │
│  NapiPPSSPP::SendMultiAxisEvent()                           │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   OhosInput 层                               │
├─────────────────────────────────────────────────────────────┤
│  HandleKeyEvent()    ──→ 按键码映射 ──→ NativeKey()         │
│  HandleAxisEvent()   ──→ 轴 ID 映射 ──→ NativeAxis()        │
│  HandleTouchEvent()  ──→ 坐标转换  ──→ NativeTouch()        │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   PPSSPP Core                                │
├─────────────────────────────────────────────────────────────┤
│  NativeKey()   ──→ g_screenManager->key()                   │
│  NativeAxis()  ──→ 手柄轴处理                                │
│  NativeTouch() ──→ g_screenManager->touch()                 │
└─────────────────────────────────────────────────────────────┘
```

## 触摸事件

触摸事件在 `ohos_xcomponent.cpp` 的 `OnTouchEvent()` 中处理：

1. XComponent 回调 `DispatchTouchEvent`
2. 获取触摸点信息 (`OH_NativeXComponent_GetTouchEvent`)
3. 坐标转换（屏幕坐标 → PPSSPP 坐标）
4. 调用 `NativeTouch()` 传递给 PPSSPP

坐标转换公式（参考 Android）：
```cpp
touch.x = touchPoint.x * display_scale_x * g_display.dpi_scale_x;
touch.y = touchPoint.y * display_scale_y * g_display.dpi_scale_y;
```

## 按键事件

### 按键码映射

OHOS 按键码与 Android 兼容，映射表在 `ohos_input.cpp` 的 `InitKeyCodeMap()` 中定义：

| 类别 | OHOS KeyCode | PPSSPP KeyCode |
|------|--------------|----------------|
| 系统键 | 2 (BACK) | NKCODE_BACK |
| 方向键 | 19-22 (DPAD) | NKCODE_DPAD_* |
| 字母键 | 29-54 (A-Z) | NKCODE_A-Z |
| 手柄键 | 96-110 | NKCODE_BUTTON_* |
| 功能键 | 131-142 (F1-F12) | NKCODE_F1-F12 |

### 设备 ID 映射

| deviceId | 设备类型 | PPSSPP DeviceID |
|----------|----------|-----------------|
| 0 | 默认 | DEVICE_ID_DEFAULT |
| 1 | 键盘 | DEVICE_ID_KEYBOARD |
| 10-19 | 手柄 0-9 | DEVICE_ID_PAD_0 - PAD_9 |

### ArkTS 使用示例

```typescript
// 在 XComponent 上监听按键事件
XComponent({ ... })
  .onKeyEvent((event: KeyEvent): void => {
    const isDown = event.type === KeyType.Down;
    ppsspp.sendKeyEvent(event.keyCode, isDown, false, 1);
  })
  .focusable(true)
```

## 手柄轴事件

### 轴 ID 映射

| OHOS AxisId | 用途 | PPSSPP AxisId |
|-------------|------|---------------|
| 0 | 左摇杆 X | JOYSTICK_AXIS_X |
| 1 | 左摇杆 Y | JOYSTICK_AXIS_Y |
| 11 | Z 轴 | JOYSTICK_AXIS_Z |
| 12 | 右摇杆 X | JOYSTICK_AXIS_RX |
| 13 | 右摇杆 Y | JOYSTICK_AXIS_RY |
| 14 | RZ 轴 | JOYSTICK_AXIS_RZ |
| 15 | 十字键 X | JOYSTICK_AXIS_HAT_X |
| 16 | 十字键 Y | JOYSTICK_AXIS_HAT_Y |
| 17 | L2 扳机 | JOYSTICK_AXIS_LTRIGGER |
| 18 | R2 扳机 | JOYSTICK_AXIS_RTRIGGER |

### ArkTS 使用示例

```typescript
// 单个轴事件
ppsspp.sendAxisEvent(0, 0, 0.5);  // 手柄0, 左摇杆X, 值0.5

// 批量轴事件（更高效）
ppsspp.sendMultiAxisEvent(0, [0, 1], [0.5, -0.3]);  // 手柄0, X和Y轴
```

## 文件清单

| 文件 | 用途 |
|------|------|
| `ohos_input.cpp` | 输入系统核心实现 |
| `ohos_input.h` | 输入系统接口声明 |
| `ohos_xcomponent.cpp` | 触摸事件处理 |
| `napi/napi_ppsspp.cpp` | NAPI 接口实现 |
| `napi/napi_ppsspp.h` | NAPI 接口声明 |
| `napi/napi_init.cpp` | NAPI 函数注册 |
| `libppsspp.d.ts` | TypeScript 类型声明 |
| `Index.ets` | ArkTS 按键事件处理示例 |

## NAPI 接口

### sendKeyEvent

```typescript
function sendKeyEvent(
  keyCode: number,      // OHOS 按键码
  isDown: boolean,      // 是否按下
  isRepeat?: boolean,   // 是否重复按键
  deviceId?: number     // 设备 ID
): boolean;             // 是否消费了按键
```

### sendAxisEvent

```typescript
function sendAxisEvent(
  deviceId: number,     // 设备 ID
  axisId: number,       // 轴 ID
  value: number         // 轴值 (-1.0 ~ 1.0)
): void;
```

### sendMultiAxisEvent

```typescript
function sendMultiAxisEvent(
  deviceId: number,     // 设备 ID
  axisIds: number[],    // 轴 ID 数组
  values: number[]      // 轴值数组
): void;
```

## 注意事项

1. **触摸事件**：主要在 XComponent 的 native 回调中处理，不需要通过 ArkTS
2. **按键事件**：需要 XComponent 设置 `.focusable(true)` 才能接收按键
3. **手柄支持**：需要 ArkTS 层监听手柄事件并调用 NAPI 接口
4. **坐标转换**：触摸坐标需要根据 DPI 缩放进行转换
