# 黑屏调试日志说明

## 问题描述
进入游戏后黑屏卡死，需要通过日志定位问题。

## 添加的调试日志位置

### 1. ohos_xcomponent.cpp - 渲染循环
- 每帧记录 NativeFrame 和 ThreadFrame 的耗时
- 如果某个操作超过 100ms，会输出警告
- 如果超过 2 秒没有日志，会强制输出（检测卡死）

关键日志标签：`PPSSPP_XComponent`

### 2. UI/NativeApp.cpp - NativeFrame
- 记录 BeginFrame、render、EndFrame、Present 的调用
- 帮助定位是哪个阶段卡住

关键日志标签：`PPSSPP_NativeApp`

### 3. UI/EmuScreen.cpp - EmuScreen::render
- 记录 PSP_RunLoopWhileState 的调用和耗时
- 记录 coreState 的变化

关键日志标签：`PPSSPP_EmuScreen`

### 4. Core/Core.cpp - Core_RunLoopUntil
- 记录核心循环的迭代次数
- 如果迭代超过 10000 次会强制退出并警告

关键日志标签：`PPSSPP_Core`

### 5. Core/HLE/sceDisplay.cpp - VBlank 处理
- 记录每次 VBlank 时的状态
- 记录 Core_NextFrame 的调用结果

关键日志标签：`PPSSPP_sceDisplay`

## 如何分析日志

1. 搜索 `PPSSPP_` 前缀的日志
2. 查看最后几条日志，确定卡在哪个阶段
3. 常见问题：
   - 如果卡在 `NativeFrame`：可能是 render 或 GPU 问题
   - 如果卡在 `PSP_RunLoopWhileState`：可能是 CPU 模拟问题
   - 如果卡在 `ThreadFrame`：可能是 GL 渲染队列问题
   - 如果没有 `VBlank` 日志：游戏可能没有正确初始化

## 日志过滤命令

```bash
# 过滤 PPSSPP 相关日志
hdc shell hilog | grep PPSSPP

# 只看警告和错误
hdc shell hilog | grep -E "(PPSSPP.*SLOW|PPSSPP.*ERROR|PPSSPP.*WARN)"
```
