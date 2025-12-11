# 鸿蒙震动功能实现

## 概述

本文档描述了 PPSSPP 在鸿蒙系统上的震动功能实现。震动功能允许游戏在特定事件（如按键反馈）时触发设备震动。

## 架构设计

震动功能采用分层架构：

```
PPSSPP Core (System_Vibrate)
         ↓
C++ 震动管理器 (OhosVibration)
         ↓
NAPI 接口 (NapiPPSSPP::Vibrate)
         ↓
ArkTS 震动回调
         ↓
鸿蒙震动 API (@ohos.vibrator)
```

## 实现细节

### 1. C++ 层

#### OhosVibration 模块
- **文件**: `ohos_vibration.h`, `ohos_vibration.cpp`
- **功能**: 提供震动功能的 C++ 接口
- **主要函数**:
  - `Initialize()`: 初始化震动系统
  - `Vibrate(int duration)`: 触发震动
  - `SetVibrationCallback()`: 设置回调函数

#### System_Vibrate 函数
- **文件**: `ohos_system_stubs.cpp`
- **功能**: PPSSPP Core 调用的震动接口
- **实现**: 调用 `OhosVibration::Vibrate()`

### 2. NAPI 层

#### 震动接口
- **文件**: `napi_ppsspp.h`, `napi_ppsspp.cpp`
- **函数**:
  - `Vibrate()`: 直接触发震动
  - `SetVibrationCallback()`: 设置 ArkTS 回调函数

### 3. ArkTS 层

#### 震动回调实现
- **文件**: `Index.ets`
- **功能**: 
  - 使用 `@ohos.vibrator` API 实现真正的震动
  - 限制震动时长范围（1-1000ms）
  - 错误处理和日志记录

## 使用方法

### 1. 权限配置

在 `module.json5` 中添加震动权限：

```json
{
  "requestPermissions": [
    {
      "name": "ohos.permission.VIBRATE"
    }
  ]
}
```

### 2. 初始化

震动系统在应用启动时自动初始化：

```cpp
// 在 OhosApp::Initialize() 中
OhosVibration::Initialize();
```

### 3. 设置回调

在 ArkTS 层设置震动回调：

```typescript
ppsspp.setVibrationCallback((duration: number): boolean => {
  // 使用鸿蒙震动 API
  vibrator.startVibration({
    type: 'time',
    duration: Math.max(1, Math.min(duration, 1000))
  }, {
    id: 0,
    usage: 'alarm'
  });
  return true;
});
```

### 4. 触发震动

从 PPSSPP Core 触发震动：

```cpp
// 在游戏代码中
System_Vibrate(100); // 震动 100ms
```

## 测试

### 手动测试

应用中包含一个测试按钮，可以手动触发震动：

```typescript
Button('Test Vibration')
  .onClick(() => {
    ppsspp.vibrate(100);
  })
```

### 游戏内测试

1. 启动 PPSSPP
2. 进入设置 → 控制 → 触摸控制
3. 启用"按键震动"选项
4. 在游戏中按虚拟按键，应该能感受到震动反馈

## 故障排除

### 震动不工作

1. **检查权限**: 确认 `ohos.permission.VIBRATE` 权限已添加
2. **检查设备**: 确认设备支持震动功能
3. **检查日志**: 查看控制台日志中的震动相关信息
4. **检查回调**: 确认震动回调已正确设置

### 常见日志

```
[PPSSPP] Setting up vibration callback
[PPSSPP] Vibration callback set successfully
[PPSSPP] Vibration requested: 100ms
[PPSSPP] Vibration started successfully: 100ms
```

## 限制

1. **震动时长**: 限制在 1-1000ms 范围内
2. **震动模式**: 目前只支持时长震动，不支持自定义震动模式
3. **并发震动**: 不支持同时多个震动请求

## 未来改进

1. 支持自定义震动模式
2. 支持震动强度调节
3. 支持更复杂的震动序列
4. 添加震动设置选项