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
 * 鸿蒙系统接口实现
 * 
 * 职责：
 * - 系统初始化和生命周期管理
 * - 文件系统操作（目录管理、文件检查）
 * - 应用数据目录管理
 * 
 * 注意：
 * - 系统属性相关功能在 ohos_system_properties.cpp 中实现
 * - 安全区域管理在 ohos_system_properties.cpp 中统一处理
 * - 系统功能桩在 ohos_system_stubs.cpp 中实现
 */

#include "ohos_system.h"
#include "ohos_hilog.h"  // 使用自定义的 hilog 包装器，避免 LogLevel 冲突
#include "Common/System/System.h"  // for System_GetProperty and SYSPROP_* constants
#include <sys/stat.h>
#include <unistd.h>

#define SYS_TAG "PPSSPP_System"

namespace OhosSystem {

static bool g_initialized = false;
static std::string g_dataDir;
static std::string g_cacheDir;


bool Initialize(const std::string& dataDir, const std::string& cacheDir) {
    if (g_initialized) {
        OHOS_LOGW(SYS_TAG, "System already initialized");
        return true;
    }
    
    if (dataDir.empty() || cacheDir.empty()) {
        OHOS_LOGE(SYS_TAG, "Invalid paths: dataDir=%{public}s, cacheDir=%{public}s", 
                  dataDir.c_str(), cacheDir.c_str());
        return false;
    }
    
    OHOS_LOGI(SYS_TAG, "Initializing system interface");
    OHOS_LOGI(SYS_TAG, "  dataDir: %{public}s", dataDir.c_str());
    OHOS_LOGI(SYS_TAG, "  cacheDir: %{public}s", cacheDir.c_str());
    
    // 保存从 ArkTS context 传入的路径
    g_dataDir = dataDir;
    g_cacheDir = cacheDir;
    
    // 确保目录存在
    CreateDirectory(g_dataDir);
    CreateDirectory(g_cacheDir);
    
    g_initialized = true;
    OHOS_LOGI(SYS_TAG, "System interface initialized");
    return true;
}

bool IsInitialized() {
    return g_initialized;
}

void Shutdown() {
    if (!g_initialized) {
        return;
    }
    
    OHOS_LOGI(SYS_TAG, "Shutting down system interface");
    g_initialized = false;
}

std::string GetDataDirectory() {
    return g_dataDir;
}

std::string GetCacheDirectory() {
    return g_cacheDir;
}

std::string GetExternalStorageDirectory() {
    // 鸿蒙中外部存储需要通过 picker 或权限申请访问
    // 默认返回应用数据目录，实际外部存储路径需要通过 ArkTS 层获取
    return g_dataDir;
}

bool FileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool CreateDirectory(const std::string& path) {
    if (FileExists(path)) {
        return true;
    }
    
    if (mkdir(path.c_str(), 0755) == 0) {
        OHOS_LOGI(SYS_TAG, "Created directory: %{public}s", path.c_str());
        return true;
    }
    
    OHOS_LOGE(SYS_TAG, "Failed to create directory: %{public}s", path.c_str());
    return false;
}



} // namespace OhosSystem

