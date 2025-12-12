/**
 * 鸿蒙输入系统实现
 * 参考 Android 实现：android/jni/app-android.cpp
 * 
 * 输入处理流程：
 * 1. 触摸事件：XComponent 回调 -> OnTouchEvent -> NativeTouch (已在 ohos_xcomponent.cpp 实现)
 * 2. 按键事件：ArkTS -> NAPI -> HandleKeyEvent -> NativeKey
 * 3. 手柄轴事件：ArkTS -> NAPI -> HandleAxisEvent -> NativeAxis
 */

#include "ohos_input.h"
#include "ohos_hilog.h"
#include "Common/Input/InputState.h"
#include "Common/Input/KeyCodes.h"
#include "Common/System/NativeApp.h"
#include "Common/System/Display.h"
#include "Common/TimeUtil.h"
#include <unordered_map>

#define INPUT_TAG "PPSSPP_Input"

namespace OhosInput {

static bool g_initialized = false;
static bool g_rendererInited = false;

// OHOS 按键码到 PPSSPP 按键码的映射表
// OHOS KeyCode 参考: https://developer.huawei.com/consumer/cn/doc/harmonyos-references/ts-appendix-enums-0000001774121374
static std::unordered_map<int, InputKeyCode> g_keyCodeMap;

// 初始化按键映射表
static void InitKeyCodeMap() {
    g_keyCodeMap.clear();
    
    // ============================================
    // 系统按键
    // ============================================
    g_keyCodeMap[2] = NKCODE_BACK;           // KEYCODE_BACK
    g_keyCodeMap[3] = NKCODE_HOME;           // KEYCODE_HOME
    g_keyCodeMap[82] = NKCODE_MENU;          // KEYCODE_MENU
    
    // ============================================
    // 方向键 (D-Pad)
    // ============================================
    g_keyCodeMap[19] = NKCODE_DPAD_UP;       // KEYCODE_DPAD_UP
    g_keyCodeMap[20] = NKCODE_DPAD_DOWN;     // KEYCODE_DPAD_DOWN
    g_keyCodeMap[21] = NKCODE_DPAD_LEFT;     // KEYCODE_DPAD_LEFT
    g_keyCodeMap[22] = NKCODE_DPAD_RIGHT;    // KEYCODE_DPAD_RIGHT
    g_keyCodeMap[23] = NKCODE_DPAD_CENTER;   // KEYCODE_DPAD_CENTER / ENTER
    
    // ============================================
    // 数字键
    // ============================================
    g_keyCodeMap[7] = NKCODE_0;
    g_keyCodeMap[8] = NKCODE_1;
    g_keyCodeMap[9] = NKCODE_2;
    g_keyCodeMap[10] = NKCODE_3;
    g_keyCodeMap[11] = NKCODE_4;
    g_keyCodeMap[12] = NKCODE_5;
    g_keyCodeMap[13] = NKCODE_6;
    g_keyCodeMap[14] = NKCODE_7;
    g_keyCodeMap[15] = NKCODE_8;
    g_keyCodeMap[16] = NKCODE_9;
    
    // ============================================
    // 字母键 (A-Z)
    // ============================================
    g_keyCodeMap[29] = NKCODE_A;
    g_keyCodeMap[30] = NKCODE_B;
    g_keyCodeMap[31] = NKCODE_C;
    g_keyCodeMap[32] = NKCODE_D;
    g_keyCodeMap[33] = NKCODE_E;
    g_keyCodeMap[34] = NKCODE_F;
    g_keyCodeMap[35] = NKCODE_G;
    g_keyCodeMap[36] = NKCODE_H;
    g_keyCodeMap[37] = NKCODE_I;
    g_keyCodeMap[38] = NKCODE_J;
    g_keyCodeMap[39] = NKCODE_K;
    g_keyCodeMap[40] = NKCODE_L;
    g_keyCodeMap[41] = NKCODE_M;
    g_keyCodeMap[42] = NKCODE_N;
    g_keyCodeMap[43] = NKCODE_O;
    g_keyCodeMap[44] = NKCODE_P;
    g_keyCodeMap[45] = NKCODE_Q;
    g_keyCodeMap[46] = NKCODE_R;
    g_keyCodeMap[47] = NKCODE_S;
    g_keyCodeMap[48] = NKCODE_T;
    g_keyCodeMap[49] = NKCODE_U;
    g_keyCodeMap[50] = NKCODE_V;
    g_keyCodeMap[51] = NKCODE_W;
    g_keyCodeMap[52] = NKCODE_X;
    g_keyCodeMap[53] = NKCODE_Y;
    g_keyCodeMap[54] = NKCODE_Z;
    
    // ============================================
    // 功能键
    // ============================================
    g_keyCodeMap[66] = NKCODE_ENTER;         // KEYCODE_ENTER
    g_keyCodeMap[67] = NKCODE_DEL;           // KEYCODE_DEL (Backspace)
    g_keyCodeMap[61] = NKCODE_TAB;           // KEYCODE_TAB
    g_keyCodeMap[62] = NKCODE_SPACE;         // KEYCODE_SPACE
    g_keyCodeMap[111] = NKCODE_ESCAPE;       // KEYCODE_ESCAPE
    
    // ============================================
    // 修饰键
    // ============================================
    g_keyCodeMap[57] = NKCODE_ALT_LEFT;      // KEYCODE_ALT_LEFT
    g_keyCodeMap[58] = NKCODE_ALT_RIGHT;     // KEYCODE_ALT_RIGHT
    g_keyCodeMap[59] = NKCODE_SHIFT_LEFT;    // KEYCODE_SHIFT_LEFT
    g_keyCodeMap[60] = NKCODE_SHIFT_RIGHT;   // KEYCODE_SHIFT_RIGHT
    g_keyCodeMap[113] = NKCODE_CTRL_LEFT;    // KEYCODE_CTRL_LEFT
    g_keyCodeMap[114] = NKCODE_CTRL_RIGHT;   // KEYCODE_CTRL_RIGHT
    
    // ============================================
    // 游戏手柄按键 (与 Android 兼容)
    // ============================================
    g_keyCodeMap[96] = NKCODE_BUTTON_A;      // KEYCODE_BUTTON_A
    g_keyCodeMap[97] = NKCODE_BUTTON_B;      // KEYCODE_BUTTON_B
    g_keyCodeMap[98] = NKCODE_BUTTON_C;      // KEYCODE_BUTTON_C
    g_keyCodeMap[99] = NKCODE_BUTTON_X;      // KEYCODE_BUTTON_X
    g_keyCodeMap[100] = NKCODE_BUTTON_Y;     // KEYCODE_BUTTON_Y
    g_keyCodeMap[101] = NKCODE_BUTTON_Z;     // KEYCODE_BUTTON_Z
    g_keyCodeMap[102] = NKCODE_BUTTON_L1;    // KEYCODE_BUTTON_L1
    g_keyCodeMap[103] = NKCODE_BUTTON_R1;    // KEYCODE_BUTTON_R1
    g_keyCodeMap[104] = NKCODE_BUTTON_L2;    // KEYCODE_BUTTON_L2
    g_keyCodeMap[105] = NKCODE_BUTTON_R2;    // KEYCODE_BUTTON_R2
    g_keyCodeMap[106] = NKCODE_BUTTON_THUMBL; // KEYCODE_BUTTON_THUMBL
    g_keyCodeMap[107] = NKCODE_BUTTON_THUMBR; // KEYCODE_BUTTON_THUMBR
    g_keyCodeMap[108] = NKCODE_BUTTON_START; // KEYCODE_BUTTON_START
    g_keyCodeMap[109] = NKCODE_BUTTON_SELECT; // KEYCODE_BUTTON_SELECT
    g_keyCodeMap[110] = NKCODE_BUTTON_MODE;  // KEYCODE_BUTTON_MODE
    
    // ============================================
    // 音量键
    // ============================================
    g_keyCodeMap[24] = NKCODE_VOLUME_UP;     // KEYCODE_VOLUME_UP
    g_keyCodeMap[25] = NKCODE_VOLUME_DOWN;   // KEYCODE_VOLUME_DOWN
    g_keyCodeMap[164] = NKCODE_VOLUME_MUTE;  // KEYCODE_VOLUME_MUTE
    
    // ============================================
    // F1-F12 功能键
    // ============================================
    g_keyCodeMap[131] = NKCODE_F1;
    g_keyCodeMap[132] = NKCODE_F2;
    g_keyCodeMap[133] = NKCODE_F3;
    g_keyCodeMap[134] = NKCODE_F4;
    g_keyCodeMap[135] = NKCODE_F5;
    g_keyCodeMap[136] = NKCODE_F6;
    g_keyCodeMap[137] = NKCODE_F7;
    g_keyCodeMap[138] = NKCODE_F8;
    g_keyCodeMap[139] = NKCODE_F9;
    g_keyCodeMap[140] = NKCODE_F10;
    g_keyCodeMap[141] = NKCODE_F11;
    g_keyCodeMap[142] = NKCODE_F12;
    
    // ============================================
    // 翻页键
    // ============================================
    g_keyCodeMap[92] = NKCODE_PAGE_UP;       // KEYCODE_PAGE_UP
    g_keyCodeMap[93] = NKCODE_PAGE_DOWN;     // KEYCODE_PAGE_DOWN
    g_keyCodeMap[122] = NKCODE_MOVE_HOME;    // KEYCODE_MOVE_HOME
    g_keyCodeMap[123] = NKCODE_MOVE_END;     // KEYCODE_MOVE_END
    g_keyCodeMap[124] = NKCODE_INSERT;       // KEYCODE_INSERT
    g_keyCodeMap[112] = NKCODE_FORWARD_DEL;  // KEYCODE_FORWARD_DEL
    
    OHOS_LOGI(INPUT_TAG, "Key code map initialized with %{public}zu entries", g_keyCodeMap.size());
}

// OHOS 轴 ID 到 PPSSPP 轴 ID 的映射
static InputAxis MapAxisId(int ohosAxisId) {
    // OHOS 轴 ID 与 Android 兼容
    switch (ohosAxisId) {
        case 0: return JOYSTICK_AXIS_X;
        case 1: return JOYSTICK_AXIS_Y;
        case 11: return JOYSTICK_AXIS_Z;
        case 12: return JOYSTICK_AXIS_RX;
        case 13: return JOYSTICK_AXIS_RY;
        case 14: return JOYSTICK_AXIS_RZ;
        case 15: return JOYSTICK_AXIS_HAT_X;
        case 16: return JOYSTICK_AXIS_HAT_Y;
        case 17: return JOYSTICK_AXIS_LTRIGGER;
        case 18: return JOYSTICK_AXIS_RTRIGGER;
        case 22: return JOYSTICK_AXIS_GAS;
        case 23: return JOYSTICK_AXIS_BRAKE;
        default: return (InputAxis)ohosAxisId;
    }
}

bool Initialize() {
    if (g_initialized) {
        return true;
    }
    
    OHOS_LOGI(INPUT_TAG, "Initializing input system");
    
    // 初始化按键映射
    InitKeyCodeMap();
    
    g_initialized = true;
    return true;
}

void Shutdown() {
    if (!g_initialized) {
        return;
    }
    
    OHOS_LOGI(INPUT_TAG, "Shutting down input system");
    g_keyCodeMap.clear();
    g_initialized = false;
    g_rendererInited = false;
}

void SetRendererInited(bool inited) {
    g_rendererInited = inited;
}

bool IsRendererInited() {
    return g_rendererInited;
}

void HandleTouchEvent(float x, float y, TouchAction action, int pointerId) {
    // 注意：触摸事件已经在 ohos_xcomponent.cpp 的 OnTouchEvent 中处理
    // 这个函数保留用于从 ArkTS 层直接发送触摸事件（如果需要）
    
    if (!g_initialized || !g_rendererInited) {
        return;
    }
    
    TouchInput touch{};
    touch.id = pointerId;
    
    // 坐标转换（参考 Android 实现）
    // 假设传入的是屏幕像素坐标
    float display_scale_x = 1.0f;
    float display_scale_y = 1.0f;
    
    touch.x = x * display_scale_x * g_display.dpi_scale_x;
    touch.y = y * display_scale_y * g_display.dpi_scale_y;
    
    // 映射动作类型
    switch (action) {
        case TouchAction::DOWN:
            touch.flags = TOUCH_DOWN;
            break;
        case TouchAction::MOVE:
            touch.flags = TOUCH_MOVE;
            break;
        case TouchAction::UP:
            touch.flags = TOUCH_UP;
            break;
        case TouchAction::CANCEL:
            touch.flags = TOUCH_CANCEL;
            break;
    }
    
    touch.timestamp = time_now_d();
    touch.buttons = 0;
    
    NativeTouch(touch);
}

bool HandleKeyEvent(int keyCode, bool isDown, bool isRepeat, int deviceId) {
    if (!g_initialized) {
        OHOS_LOGW(INPUT_TAG, "Input system not initialized");
        return false;
    }
    
    // 查找按键映射
    auto it = g_keyCodeMap.find(keyCode);
    InputKeyCode mappedKeyCode;
    
    if (it != g_keyCodeMap.end()) {
        mappedKeyCode = it->second;
    } else {
        // 未映射的按键，直接使用原始值（如果在有效范围内）
        if (keyCode > 0 && keyCode < NKCODE_MAX) {
            mappedKeyCode = (InputKeyCode)keyCode;
        } else {
            OHOS_LOGD(INPUT_TAG, "Unknown key code: %{public}d", keyCode);
            return false;
        }
    }
    
    // 确定设备 ID
    InputDeviceID inputDeviceId;
    if (deviceId >= 10 && deviceId <= 19) {
        // 手柄设备
        inputDeviceId = (InputDeviceID)(DEVICE_ID_PAD_0 + (deviceId - 10));
    } else if (deviceId == 1) {
        inputDeviceId = DEVICE_ID_KEYBOARD;
    } else {
        inputDeviceId = DEVICE_ID_DEFAULT;
    }
    
    // 忽略手柄的 keyCode 0（某些手柄在按 L2/R2 时会产生）
    if (mappedKeyCode == 0 && inputDeviceId >= DEVICE_ID_PAD_0 && inputDeviceId <= DEVICE_ID_PAD_9) {
        return true;  // 吃掉这个按键
    }
    
    KeyInput keyInput;
    keyInput.deviceId = inputDeviceId;
    keyInput.keyCode = mappedKeyCode;
    keyInput.flags = isDown ? KEY_DOWN : KEY_UP;
    
    if (isRepeat && isDown) {
        keyInput.flags |= KEY_IS_REPEAT;
    }
    
    OHOS_LOGD(INPUT_TAG, "Key event: ohos=%{public}d -> ppsspp=%{public}d, down=%{public}d, device=%{public}d",
              keyCode, (int)mappedKeyCode, isDown, (int)inputDeviceId);
    
    return NativeKey(keyInput);
}

void HandleAxisEvent(int deviceId, int axisId, float value) {
    if (!g_initialized) {
        return;
    }
    
    // 确定设备 ID
    InputDeviceID inputDeviceId;
    if (deviceId >= 10 && deviceId <= 19) {
        inputDeviceId = (InputDeviceID)(DEVICE_ID_PAD_0 + (deviceId - 10));
    } else if (deviceId >= 0 && deviceId < 10) {
        inputDeviceId = (InputDeviceID)(DEVICE_ID_PAD_0 + deviceId);
    } else {
        inputDeviceId = DEVICE_ID_PAD_0;
    }
    
    AxisInput axis;
    axis.deviceId = inputDeviceId;
    axis.axisId = MapAxisId(axisId);
    axis.value = value;
    
    OHOS_LOGD(INPUT_TAG, "Axis event: device=%{public}d, axis=%{public}d->%{public}d, value=%{public}.3f",
              deviceId, axisId, (int)axis.axisId, value);
    
    NativeAxis(&axis, 1);
}

void HandleMultiAxisEvent(int deviceId, const int* axisIds, const float* values, int count) {
    if (!g_initialized || count <= 0) {
        return;
    }
    
    // 确定设备 ID
    InputDeviceID inputDeviceId;
    if (deviceId >= 10 && deviceId <= 19) {
        inputDeviceId = (InputDeviceID)(DEVICE_ID_PAD_0 + (deviceId - 10));
    } else if (deviceId >= 0 && deviceId < 10) {
        inputDeviceId = (InputDeviceID)(DEVICE_ID_PAD_0 + deviceId);
    } else {
        inputDeviceId = DEVICE_ID_PAD_0;
    }
    
    // 创建轴输入数组
    AxisInput* axes = new AxisInput[count];
    for (int i = 0; i < count; i++) {
        axes[i].deviceId = inputDeviceId;
        axes[i].axisId = MapAxisId(axisIds[i]);
        axes[i].value = values[i];
    }
    
    NativeAxis(axes, count);
    delete[] axes;
}

} // namespace OhosInput
