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
        
        case SYSPROP_SUPPORTS_SHARE_TEXT:
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

// 前向声明 - 来自 napi_ppsspp.cpp
namespace NapiPPSSPP {
    bool BrowseForImage(int requestId);
    bool BrowseForFile(int requestId, int fileType);
    bool BrowseForFolder(int requestId);
    bool ShowInputTextDialog(int requestId, const std::string& title, const std::string& defaultText);
    bool CopyToClipboard(const std::string& text);
    bool SetKeepScreenOn(bool keepOn);
    bool ShareText(const std::string& text);
    bool ShowFileInFolder(const std::string& path);
    bool RestartApp(const std::string& params);
    bool ExitApp();
    bool RecreateActivity();
}

// 实现 System_MakeRequest 函数
bool System_MakeRequest(
    SystemRequestType type,
    int requestId,
    const std::string &param1,
    const std::string &param2,
    int64_t param3,
    int64_t param4) {
    
    switch (type) {
        case SystemRequestType::BROWSE_FOR_IMAGE:
            // 调用 NAPI 层的图片选择器
            INFO_LOG(Log::System, "System_MakeRequest: BROWSE_FOR_IMAGE, requestId=%d", requestId);
            return NapiPPSSPP::BrowseForImage(requestId);
        
        case SystemRequestType::BROWSE_FOR_FILE:
            // 调用 NAPI 层的文件选择器
            // param3 包含 BrowseFileType
            INFO_LOG(Log::System, "System_MakeRequest: BROWSE_FOR_FILE, requestId=%d, fileType=%d", requestId, (int)param3);
            return NapiPPSSPP::BrowseForFile(requestId, (int)param3);
        
        case SystemRequestType::BROWSE_FOR_FOLDER:
            // 调用 NAPI 层的文件夹选择器
            INFO_LOG(Log::System, "System_MakeRequest: BROWSE_FOR_FOLDER, requestId=%d", requestId);
            return NapiPPSSPP::BrowseForFolder(requestId);
        
        case SystemRequestType::INPUT_TEXT_MODAL:
            // 调用 NAPI 层的文本输入对话框
            // param1 = 标题, param2 = 默认文本
            INFO_LOG(Log::System, "System_MakeRequest: INPUT_TEXT_MODAL, requestId=%d, title=%s", requestId, param1.c_str());
            return NapiPPSSPP::ShowInputTextDialog(requestId, param1, param2);
        
        case SystemRequestType::COPY_TO_CLIPBOARD:
            // 复制文本到剪贴板
            INFO_LOG(Log::System, "System_MakeRequest: COPY_TO_CLIPBOARD");
            return NapiPPSSPP::CopyToClipboard(param1);
        
        case SystemRequestType::SET_KEEP_SCREEN_BRIGHT:
            // 设置屏幕常亮
            // param3 非零表示保持常亮
            INFO_LOG(Log::System, "System_MakeRequest: SET_KEEP_SCREEN_BRIGHT, keepOn=%d", (int)(param3 != 0));
            return NapiPPSSPP::SetKeepScreenOn(param3 != 0);
        
        case SystemRequestType::SHARE_TEXT:
            // 分享文本
            INFO_LOG(Log::System, "System_MakeRequest: SHARE_TEXT");
            return NapiPPSSPP::ShareText(param1);
        
        case SystemRequestType::SHOW_FILE_IN_FOLDER:
            // 显示文件位置
            INFO_LOG(Log::System, "System_MakeRequest: SHOW_FILE_IN_FOLDER, path=%s", param1.c_str());
            return NapiPPSSPP::ShowFileInFolder(param1);
        
        case SystemRequestType::RESTART_APP:
            // 重启应用（切换渲染引擎后需要）
            INFO_LOG(Log::System, "System_MakeRequest: RESTART_APP, params=%s", param1.c_str());
            return NapiPPSSPP::RestartApp(param1);
        
        case SystemRequestType::EXIT_APP:
            // 退出应用
            INFO_LOG(Log::System, "System_MakeRequest: EXIT_APP");
            return NapiPPSSPP::ExitApp();
        
        case SystemRequestType::RECREATE_ACTIVITY:
            // 重建 Activity（用于显示设置变更，如分辨率）
            INFO_LOG(Log::System, "System_MakeRequest: RECREATE_ACTIVITY, params=%s", param1.c_str());
            return NapiPPSSPP::RecreateActivity();
        
        default:
            // 其他请求暂不实现
            WARN_LOG(Log::System, "System_MakeRequest not implemented: type=%d", (int)type);
            return false;
    }
}
