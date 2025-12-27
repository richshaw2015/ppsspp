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
 * 鸿蒙震动功能实现
 */

#include "ohos_vibration.h"
#include "ohos_hilog.h"

#define VIBRATION_TAG "PPSSPP_Vibration"

namespace OhosVibration {

static bool g_initialized = false;
static VibrationCallback g_vibrationCallback = nullptr;

bool Initialize() {
    if (g_initialized) {
        return true;
    }
    
    g_initialized = true;
    return true;
}

void Shutdown() {
    if (!g_initialized) {
        return;
    }
    
    g_vibrationCallback = nullptr;
    g_initialized = false;
}

void SetVibrationCallback(VibrationCallback callback) {
    g_vibrationCallback = callback;
}

bool Vibrate(int duration) {
    if (!g_initialized) {
        // 自动初始化震动系统
        if (!Initialize()) {
            OHOS_LOGE(VIBRATION_TAG, "Failed to initialize vibration system");
            return false;
        }
    }
    
    // 处理 PPSSPP 的特殊震动类型（负数）
    int actualDuration = duration;
    if (duration < 0) {
        // 将 PPSSPP 的特殊震动类型转换为实际的震动时长
        switch (duration) {
            case -1: // HAPTIC_SOFT_KEYBOARD
                actualDuration = 30;
                break;
            case -2: // HAPTIC_VIRTUAL_KEY
                actualDuration = 50;
                break;
            case -3: // HAPTIC_LONG_PRESS_ACTIVATED
                actualDuration = 100;
                break;
            default:
                actualDuration = 50; // 默认震动时长
                break;
        }
    } else if (duration == 0) {
        // 零值表示停止震动，我们忽略它
        return true;
    }
    
    if (g_vibrationCallback) {
        return g_vibrationCallback(actualDuration);
    } else {
        return false;
    }
}

bool IsVibrationSupported() {
    return g_initialized && (g_vibrationCallback != nullptr);
}

} // namespace OhosVibration
