/**
 * OHOS 离线模式配置
 * 
 * 禁用需要互联网连接的功能，但保留局域网功能（本地文件传输、Ad-hoc 联机）
 */

#ifndef OHOS_OFFLINE_CONFIG_H
#define OHOS_OFFLINE_CONFIG_H

/**
 * 应用 OHOS 离线模式配置
 * 
 * 禁用的功能：
 * - RetroAchievements 成就系统（需要连接 retroachievements.org）
 * - Discord 集成（需要连接 Discord 服务器）
 * - 错误报告系统（需要连接 report.ppsspp.org）
 * - 自动下载 infra-dns.json（需要连接 ppsspp.org）
 * 
 * 保留的功能：
 * - 本地 Ad-hoc 联机（局域网内 PSP 游戏联机）
 * - 本地 WebServer（局域网内文件传输）
 * - UPnP 端口映射（用于局域网联机）
 * - Remote ISO（局域网内加载 ISO）
 * 
 * 应该在 g_Config.Load() 之后调用
 */
void OhosOfflineConfig_Apply();

#endif // OHOS_OFFLINE_CONFIG_H
