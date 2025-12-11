/**
 * 鸿蒙系统接口
 * 提供文件系统、权限等系统级功能
 */

#ifndef OHOS_SYSTEM_H
#define OHOS_SYSTEM_H

#include <string>

namespace OhosSystem {

/**
 * 初始化系统接口（带路径参数）
 * @param dataDir 应用数据目录 (context.filesDir)
 * @param cacheDir 应用缓存目录 (context.cacheDir)
 */
bool Initialize(const std::string& dataDir, const std::string& cacheDir);

/**
 * 检查是否已初始化
 */
bool IsInitialized();

/**
 * 关闭系统接口
 */
void Shutdown();

/**
 * 获取应用数据目录
 */
std::string GetDataDirectory();

/**
 * 获取应用缓存目录
 */
std::string GetCacheDirectory();

/**
 * 获取外部存储目录
 */
std::string GetExternalStorageDirectory();

/**
 * 检查文件是否存在
 */
bool FileExists(const std::string& path);

/**
 * 创建目录
 */
bool CreateDirectory(const std::string& path);

/**
 * 获取系统语言
 */
std::string GetSystemLanguage();

/**
 * 获取设备型号
 */
std::string GetDeviceModel();

/**
 * 设置安全区域 insets（从 ArkTS 层调用）
 * @param left 左侧安全区域（像素）
 * @param top 顶部安全区域（像素）
 * @param right 右侧安全区域（像素）
 * @param bottom 底部安全区域（像素）
 */
void SetSafeInsets(float left, float top, float right, float bottom);

/**
 * 获取安全区域 insets
 */
void GetSafeInsets(float& left, float& top, float& right, float& bottom);

} // namespace OhosSystem

#endif // OHOS_SYSTEM_H
