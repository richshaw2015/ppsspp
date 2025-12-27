# OHOS 离线模式配置

## 概述

PPSSPP OHOS 移植版采用离线模式配置，禁用需要互联网连接的功能，但保留局域网功能以支持本地文件传输和 PSP 联机游戏。

## 禁用的功能

以下功能在 OHOS 平台上被禁用，因为它们需要连接互联网服务器：

| 功能 | 配置项 | 原因 |
|------|--------|------|
| RetroAchievements 成就系统 | `bAchievementsEnable = false` | 需要连接 retroachievements.org |
| Discord 集成 | `bDiscordRichPresence = false` | 需要连接 Discord 服务器 |
| 错误报告系统 | `sReportHost = ""` | 需要连接 report.ppsspp.org |
| 自动下载 infra-dns.json | `bDontDownloadInfraJson = true` | 需要连接 ppsspp.org |
| 启动时自动启动远程调试器 | `bRemoteDebuggerOnStartup = false` | 默认不自动启动 |
| 启动时自动启动远程共享 | `bRemoteShareOnStartup = false` | 默认不自动启动 |
| Homebrew Store 商店 | UI 中已禁用 | 需要连接 store.ppsspp.org |

## UI 隐藏的选项

以下 UI 选项在 OHOS 平台上被隐藏：

| 选项 | 位置 | 文件 |
|------|------|------|
| RetroAchievements 设置 | 设置 → 工具 | `UI/GameSettingsScreen.cpp` |
| 启用兼容性报告反馈 | 设置 → 系统 | `UI/GameSettingsScreen.cpp` |
| Report Feedback 按钮 | 游戏暂停菜单 | `UI/PauseScreen.cpp` |
| Homebrew & Demos 标签页 | 主界面 | `UI/MainScreen.cpp` |

## 保留的局域网功能

以下功能保留，因为它们可以在局域网内使用，不需要互联网连接：

| 功能 | 配置项 | 用途 |
|------|--------|------|
| PSP Ad-hoc 联机 | `bEnableWlan` | 局域网内 PSP 游戏联机 |
| UPnP 端口映射 | `bEnableUPnP` | 自动配置路由器端口转发 |
| Remote ISO | Remote ISO 相关设置 | 局域网内加载 ISO 文件 |
| WebServer 文件传输 | WebServer 相关设置 | 局域网内传输文件 |
| 远程调试器 | 可手动启用 | 局域网内调试 |

## 实现细节

### 文件位置

- 头文件: `ohos/entry/src/main/cpp/ohos_offline_config.h`
- 实现文件: `ohos/entry/src/main/cpp/ohos_offline_config.cpp`

### 调用时机

离线配置在 `UI/NativeApp.cpp` 的 `NativeInit()` 函数中，`g_Config.Load()` 之后被调用：

```cpp
#if PPSSPP_PLATFORM(OHOS)
    // 应用 OHOS 离线模式配置
    OhosOfflineConfig_Apply();
#endif
```

### 日志输出

配置应用时会输出详细日志，标签为 `PPSSPP_Offline`：

```
=== Applying OHOS Offline Mode Configuration ===
Disabled: RetroAchievements (bAchievementsEnable = false)
Disabled: Discord Rich Presence (bDiscordRichPresence = false)
Disabled: Error Reporting (sReportHost = "")
Disabled: Auto-download infra-dns.json (bDontDownloadInfraJson = true)
Disabled: Remote Debugger on Startup (bRemoteDebuggerOnStartup = false)
Disabled: Remote Share on Startup (bRemoteShareOnStartup = false)
Preserved: WLAN/Ad-hoc (bEnableWlan = ...)
Preserved: UPnP (bEnableUPnP = ...)
Preserved: Remote ISO settings (for LAN use)
Preserved: WebServer settings (for LAN file transfer)
=== OHOS Offline Mode Configuration Applied ===
```

## 网络权限

`module.json5` 中仍然声明了 `ohos.permission.INTERNET` 权限，这是为了支持局域网功能：

```json
"requestPermissions": [
  { "name": "ohos.permission.INTERNET" },
  { "name": "ohos.permission.VIBRATE" }
]
```

如果要完全禁用网络功能，可以移除此权限声明。

## 用户可配置

用户仍然可以在设置中手动启用/禁用局域网功能：

- 网络设置 → 启用 WLAN
- 网络设置 → 启用 UPnP
- 工具 → Remote ISO
- 工具 → 远程调试器

这些设置会被保存，但需要互联网的功能（如成就系统）即使用户尝试启用也不会工作。
