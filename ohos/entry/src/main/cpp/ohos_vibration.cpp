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