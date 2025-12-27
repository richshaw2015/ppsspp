/**
 * OHOS 离线模式配置实现
 * 
 * 禁用需要互联网连接的功能，但保留局域网功能
 */

#include "ohos_offline_config.h"
#include "ohos_hilog.h"
#include "Core/Config.h"

#define OFFLINE_TAG "PPSSPP_Offline"

void OhosOfflineConfig_Apply() {
    OHOS_LOGI(OFFLINE_TAG, "=== Applying OHOS Offline Mode Configuration ===");
    
    // ========== 禁用需要互联网的功能 ==========
    
    // 1. 禁用 RetroAchievements 成就系统
    // 需要连接 retroachievements.org 服务器
    g_Config.bAchievementsEnable = false;
    OHOS_LOGI(OFFLINE_TAG, "Disabled: RetroAchievements (bAchievementsEnable = false)");
    
    // 2. 禁用 Discord 集成
    // 需要连接 Discord 服务器
    g_Config.bDiscordRichPresence = false;
    OHOS_LOGI(OFFLINE_TAG, "Disabled: Discord Rich Presence (bDiscordRichPresence = false)");
    
    // 3. 禁用错误报告系统
    // 需要连接 report.ppsspp.org
    g_Config.sReportHost = "";
    OHOS_LOGI(OFFLINE_TAG, "Disabled: Error Reporting (sReportHost = \"\")");
    
    // 4. 禁用自动下载 infra-dns.json
    // 需要连接 ppsspp.org
    g_Config.bDontDownloadInfraJson = true;
    OHOS_LOGI(OFFLINE_TAG, "Disabled: Auto-download infra-dns.json (bDontDownloadInfraJson = true)");
    
    // 5. 禁用启动时自动启动远程调试器
    // 远程调试器本身可以在局域网使用，但默认不自动启动
    g_Config.bRemoteDebuggerOnStartup = false;
    OHOS_LOGI(OFFLINE_TAG, "Disabled: Remote Debugger on Startup (bRemoteDebuggerOnStartup = false)");
    
    // 6. 禁用启动时自动启动远程共享
    // 远程共享本身可以在局域网使用，但默认不自动启动
    g_Config.bRemoteShareOnStartup = false;
    OHOS_LOGI(OFFLINE_TAG, "Disabled: Remote Share on Startup (bRemoteShareOnStartup = false)");
    
    // ========== 保留局域网功能 ==========
    
    // 以下功能保持用户设置，因为它们可以在局域网内使用：
    // - bEnableWlan: PSP Ad-hoc 联机（局域网）
    // - bEnableUPnP: UPnP 端口映射（用于局域网联机）
    // - Remote ISO 相关设置：可以在局域网内加载 ISO
    // - WebServer 相关设置：可以在局域网内传输文件
    
    OHOS_LOGI(OFFLINE_TAG, "Preserved: WLAN/Ad-hoc (bEnableWlan = %s)", 
              g_Config.bEnableWlan ? "true" : "false");
    OHOS_LOGI(OFFLINE_TAG, "Preserved: UPnP (bEnableUPnP = %s)", 
              g_Config.bEnableUPnP ? "true" : "false");
    OHOS_LOGI(OFFLINE_TAG, "Preserved: Remote ISO settings (for LAN use)");
    OHOS_LOGI(OFFLINE_TAG, "Preserved: WebServer settings (for LAN file transfer)");
    
    OHOS_LOGI(OFFLINE_TAG, "=== OHOS Offline Mode Configuration Applied ===");
}
