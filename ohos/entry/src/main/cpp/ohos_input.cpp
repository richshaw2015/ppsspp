/**
 * 鸿蒙输入系统实现
 */

#include "ohos_input.h"
#include "ohos_hilog.h"  // 使用自定义的 hilog 包装器，避免 LogLevel 冲突

#define INPUT_TAG "PPSSPP_Input"

namespace OhosInput {

static bool g_initialized = false;

bool Initialize() {
    if (g_initialized) {
        return true;
    }
    
    OHOS_LOGI(INPUT_TAG, "Initializing input system");
    
    // TODO: 初始化输入系统
    // 1. 设置触摸输入处理
    // 2. 设置按键映射
    // 3. 初始化手柄支持
    
    g_initialized = true;
    return true;
}

void Shutdown() {
    if (!g_initialized) {
        return;
    }
    
    OHOS_LOGI(INPUT_TAG, "Shutting down input system");
    g_initialized = false;
}

void HandleTouchEvent(float x, float y, TouchAction action, int pointerId) {
    if (!g_initialized) {
        return;
    }
    
    OHOS_LOGD(INPUT_TAG, "Touch event: x=%{public}f, y=%{public}f, action=%{public}d, id=%{public}d",
              x, y, static_cast<int>(action), pointerId);
    
    // TODO: 转换为 PPSSPP 输入事件
    // 1. 转换坐标系
    // 2. 处理虚拟按键
    // 3. 传递到 Core
}

void HandleKeyEvent(int keyCode, bool isDown) {
    if (!g_initialized) {
        return;
    }
    
    OHOS_LOGD(INPUT_TAG, "Key event: code=%{public}d, down=%{public}d", keyCode, isDown);
    
    // TODO: 转换为 PPSSPP 按键事件
}

void HandleAxisEvent(int axis, float value) {
    if (!g_initialized) {
        return;
    }
    
    OHOS_LOGD(INPUT_TAG, "Axis event: axis=%{public}d, value=%{public}f", axis, value);
    
    // TODO: 处理手柄轴事件
}

} // namespace OhosInput
