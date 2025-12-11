# 震动功能调试指南

## 问题现象
PPSSPP 鸿蒙版本的震动功能不工作，按键震动没有反应。

## 调试步骤

### 1. 检查权限配置
确认 `module.json5` 中已添加震动权限：
```json
{
  "requestPermissions": [
    {
      "name": "ohos.permission.VIBRATE"
    }
  ]
}
```

### 2. 测试基本震动功能
使用应用中的"Test Vibration"按钮测试：
- 点击按钮
- 查看控制台日志
- 检查是否有震动反馈

### 3. 检查日志输出
关键日志信息：
```
[PPSSPP] Testing simple vibration...
[PPSSPP] Short vibration success
```
或
```
[PPSSPP] Short vibration failed: [错误信息]
[PPSSPP] Time vibration success
```

### 4. 常见问题排查

#### 4.1 权限问题
如果看到权限相关错误：
- 检查应用是否已授权震动权限
- 在设置中手动授权应用权限

#### 4.2 API 兼容性问题
不同鸿蒙版本的震动 API 可能不同：
- API 9: `vibrator.startVibration('short')`
- API 10+: `vibrator.startVibration({type: 'time', duration: 500})`

#### 4.3 设备支持问题
某些设备可能不支持震动：
- 检查设备是否有震动马达
- 检查系统设置中震动是否开启

### 5. 测试不同的震动方式

#### 5.1 预设震动
```typescript
vibrator.startVibration('short')
vibrator.startVibration('long')
```

#### 5.2 时长震动
```typescript
vibrator.startVibration({
  type: 'time',
  duration: 500
})
```

#### 5.3 效果震动
```typescript
vibrator.startVibration({
  type: 'preset',
  effectId: 'haptic.clock.timer',
  count: 1
})
```

### 6. NAPI 调用链调试

#### 6.1 ArkTS 层
- 检查 `setupVibrationCallback()` 是否被调用
- 检查回调函数是否正确设置

#### 6.2 NAPI 层
- 检查 `SetVibrationCallback` 是否成功
- 检查 `Vibrate` 函数是否被调用

#### 6.3 C++ 层
- 检查 `System_Vibrate` 是否被调用
- 检查 `OhosVibration::Vibrate` 是否执行

### 7. 日志关键字
搜索以下关键字来跟踪震动调用：
- `[PPSSPP] Vibration`
- `System_Vibrate`
- `NAPI Vibrate`
- `SetVibrationCallback`

### 8. 解决方案

#### 8.1 如果基本震动不工作
1. 检查设备震动设置
2. 尝试其他应用的震动功能
3. 检查鸿蒙版本兼容性

#### 8.2 如果 NAPI 调用失败
1. 检查 NAPI 函数注册
2. 检查 TypeScript 类型定义
3. 检查回调函数设置

#### 8.3 如果 C++ 层不被调用
1. 检查 `System_Vibrate` 实现
2. 检查 PPSSPP 设置中的震动选项
3. 检查游戏是否触发震动事件

## 当前实现状态

### 已实现
- ✅ 震动权限配置
- ✅ C++ 震动管理器
- ✅ NAPI 接口
- ✅ ArkTS 震动回调
- ✅ 测试按钮

### 待调试
- ❓ 基本震动功能是否工作
- ❓ NAPI 回调机制是否正常
- ❓ PPSSPP Core 是否调用震动

### 下一步
1. 确认基本震动功能
2. 修复 NAPI 回调问题
3. 测试游戏内震动效果