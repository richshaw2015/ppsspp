// HarmonyOS 系统属性实现

#include "Common/System/System.h"
#include "Common/System/Request.h"
#include "Common/System/Display.h"
#include "Common/Log.h"
#include "ohos_system.h"
#include <string>

// Git 版本信息
const char *PPSSPP_GIT_VERSION = "v1.18.0-ohos";

// 实现 System_GetProperty 函数
std::string System_GetProperty(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_NAME:
            return "HarmonyOS";
        
        case SYSPROP_SYSTEMBUILD:
            return "HarmonyOS NEXT";
        
        case SYSPROP_LANGREGION:
            return "zh_CN";  // 默认中文
        
        case SYSPROP_CPUINFO:
            return "ARM64";
        
        case SYSPROP_BOARDNAME:
            return "HarmonyOS Device";
        
        case SYSPROP_GPUDRIVER_VERSION:
            return "OpenGL ES 3.2";
        
        case SYSPROP_BUILD_VERSION:
            return "1.0.0";
        
        case SYSPROP_COMPUTER_NAME:
            return "HarmonyOS";
        
        case SYSPROP_CLIPBOARD_TEXT:
            return "";  // TODO: 实现剪贴板
        
        case SYSPROP_ADDITIONAL_STORAGE_DIRS:
            return "";
        
        case SYSPROP_TEMP_DIRS:
            // 使用 OhosSystem 获取的缓存目录
            return OhosSystem::GetCacheDirectory();
        
        // Boolean properties (返回 "1" 或 "0")
        case SYSPROP_HAS_FILE_BROWSER:
            return "1";
        
        case SYSPROP_HAS_FOLDER_BROWSER:
            return "1";
        
        case SYSPROP_HAS_IMAGE_BROWSER:
            return "1";
        
        case SYSPROP_HAS_BACK_BUTTON:
            return "1";
        
        case SYSPROP_HAS_KEYBOARD:
            return "1";
        
        case SYSPROP_KEYBOARD_IS_SOFT:
            return "1";  // 软键盘
        
        case SYSPROP_HAS_ACCELEROMETER:
            return "1";
        
        case SYSPROP_HAS_OPEN_DIRECTORY:
            return "1";
        
        case SYSPROP_HAS_LOGIN_DIALOG:
            return "0";
        
        case SYSPROP_HAS_TEXT_CLIPBOARD:
            return "1";
        
        case SYSPROP_HAS_TEXT_INPUT_DIALOG:
            return "1";
        
        case SYSPROP_CAN_CREATE_SHORTCUT:
            return "0";
        
        case SYSPROP_CAN_SHOW_FILE:
            return "1";
        
        case SYSPROP_SUPPORTS_HTTPS:
            return "0";  // 暂时禁用 HTTPS
        
        case SYSPROP_DEBUGGER_PRESENT:
            return "0";
        
        case SYSPROP_HAS_ADDITIONAL_STORAGE:
            return "0";
        
        case SYSPROP_APP_GOLD:
            return "0";
        
        // Integer properties
        case SYSPROP_SYSTEMVERSION:
            return "5";  // HarmonyOS NEXT
        
        case SYSPROP_DISPLAY_XRES:
            return "1080";  // 默认分辨率
        
        case SYSPROP_DISPLAY_YRES:
            return "2340";
        
        case SYSPROP_DISPLAY_REFRESH_RATE:
            return "60";
        
        case SYSPROP_DISPLAY_LOGICAL_DPI:
            return "320";
        
        case SYSPROP_DISPLAY_DPI:
            return "480";
        
        case SYSPROP_DISPLAY_COUNT:
            return "1";
        
        case SYSPROP_MOGA_VERSION:
            return "0";
        
        case SYSPROP_AUDIO_SAMPLE_RATE:
            return "48000";
        
        case SYSPROP_AUDIO_FRAMES_PER_BUFFER:
            return "512";
        
        case SYSPROP_AUDIO_OPTIMAL_SAMPLE_RATE:
            return "48000";
        
        case SYSPROP_AUDIO_OPTIMAL_FRAMES_PER_BUFFER:
            return "512";
        
        // Float properties - 安全区域
        case SYSPROP_DISPLAY_SAFE_INSET_LEFT:
        case SYSPROP_DISPLAY_SAFE_INSET_RIGHT:
        case SYSPROP_DISPLAY_SAFE_INSET_TOP:
        case SYSPROP_DISPLAY_SAFE_INSET_BOTTOM: {
            float left, top, right, bottom;
            OhosSystem::GetSafeInsets(left, top, right, bottom);
            
            // 需要乘以 DPI 缩放，因为 PPSSPP 期望的是 DP 单位
            // 参考 Android 实现：display_scale_x * g_display.dpi_scale_x
            float value = 0.0f;
            switch (prop) {
                case SYSPROP_DISPLAY_SAFE_INSET_LEFT:
                    value = left * g_display.dpi_scale_x;
                    break;
                case SYSPROP_DISPLAY_SAFE_INSET_RIGHT:
                    value = right * g_display.dpi_scale_x;
                    break;
                case SYSPROP_DISPLAY_SAFE_INSET_TOP:
                    value = top * g_display.dpi_scale_y;
                    break;
                case SYSPROP_DISPLAY_SAFE_INSET_BOTTOM:
                    value = bottom * g_display.dpi_scale_y;
                    break;
            }
            
            char buf[32];
            snprintf(buf, sizeof(buf), "%.1f", value);
            return buf;
        }
        
        case SYSPROP_DEVICE_TYPE:
            return "0";  // DEVICE_TYPE_MOBILE
        
        default:
            WARN_LOG(Log::System, "Unknown system property: %d", prop);
            return "";
    }
}


// 实现 System_GetPropertyBool 函数
bool System_GetPropertyBool(SystemProperty prop) {
    std::string value = System_GetProperty(prop);
    return value == "1" || value == "true";
}

// 实现 System_GetPropertyFloat 函数
float System_GetPropertyFloat(SystemProperty prop) {
    std::string value = System_GetProperty(prop);
    return std::stof(value);
}

// 实现 System_MakeRequest 函数
bool System_MakeRequest(
    SystemRequestType type,
    int requestId,
    const std::string &param1,
    const std::string &param2,
    int64_t param3,
    int64_t param4) {
    // HarmonyOS 暂不实现系统请求
    // 这些请求包括：打开浏览器、分享、输入框等
    WARN_LOG(Log::System, "System_MakeRequest not implemented: type=%d", (int)type);
    return false;  // 返回 false 表示请求失败
}
