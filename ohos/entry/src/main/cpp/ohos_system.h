// PPSSPP - OpenHarmony Platform Adaptation
// Copyright (c) 2024-2025 richshaw
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License 2.0 for more details.
//
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

/**
 * 鸿蒙系统接口头文件
 * 
 * 文件组织：
 * - ohos_system.cpp: 系统初始化、文件系统操作
 * - ohos_system_properties.cpp: 系统属性、安全区域管理
 * - ohos_system_stubs.cpp: 系统功能桩实现
 * 
 * 提供统一的系统级功能接口
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


} // namespace OhosSystem

// ========== 系统属性初始化函数（在全局命名空间中）==========

/**
 * 初始化系统属性（从 ArkTS 层调用）
 * @param deviceName 设备名称
 * @param deviceBuild 系统版本字符串
 * @param language 语言区域代码（如 "zh_CN"）
 * @param boardName 主板型号/硬件型号
 * @param osVersion 系统版本号
 * @param devType 设备类型（0=手机, 1=平板, 2=电视等）
 * @param xres 屏幕宽度（像素）
 * @param yres 屏幕高度（像素）
 * @param dpi 屏幕 DPI
 * @param refreshRate 刷新率（Hz）
 */
extern "C" void OhosSystemProperties_Init(
    const char* deviceName,
    const char* deviceBuild,
    const char* language,
    const char* boardName,
    int osVersion,
    int devType,
    int xres, int yres, int dpi, float refreshRate);

/**
 * 设置安全区域（从 ArkTS 层调用）
 */
extern "C" void OhosSystemProperties_SetSafeInsets(float left, float top, float right, float bottom);

/**
 * 设置音频配置（从 ArkTS 层调用）
 */
extern "C" void OhosSystemProperties_SetAudioConfig(int rate, int frames, int optRate, int optFrames);

#endif // OHOS_SYSTEM_H
