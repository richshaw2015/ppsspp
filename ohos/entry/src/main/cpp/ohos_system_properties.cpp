// HarmonyOS 系统属性实现

#include "Common/System/System.h"
#include "Common/System/Request.h"
#include "Common/System/Display.h"
#include "Common/Log.h"
#include "ohos_system.h"
#include <string>
#include <vector>

// Git 版本信息
const char *PPSSPP_GIT_VERSION = "1.19.4";

// ========== 全局变量：动态系统属性（参考 Android 实现）==========
static std::string systemName = "HarmonyOS";
static std::string systemBuild = "HarmonyOS NEXT";
static std::string langRegion = "zh_CN";
static std::string boardName = "HarmonyOS Device";
static std::string cpuInfo = "ARM64";
static std::string gpuDriverVersion = "OpenGL ES 3.2";

static int systemVersion = 5;  // HarmonyOS NEXT
static int deviceType = 0;     // DEVICE_TYPE_MOBILE

// 显示属性
static int display_xres = 1080;
static int display_yres = 1920;
static int display_dpi = 480;
static int display_logical_dpi = 320;
static float display_refresh_rate = 60.0f;
static int display_count = 1;

// 音频属性
static int sampleRate = 48000;
static int framesPerBuffer = 512;
static int optimalSampleRate = 48000;
static int optimalFramesPerBuffer = 512;

// 安全区域（刘海屏等）
static float g_safeInsetLeft = 0.0f;
static float g_safeInsetRight = 0.0f;
static float g_safeInsetTop = 0.0f;
static float g_safeInsetBottom = 0.0f;

// 额外存储目录
static std::vector<std::string> g_additionalStorageDirs;

// ========== 初始化函数：从 ArkTS 层设置系统属性 ==========
void OhosSystemProperties_Init(
    const char* deviceName,
    const char* deviceBuild,
    const char* language,
    int osVersion,
    int devType,
    int xres, int yres, int dpi, float refreshRate) {
    
    if (deviceName) systemName = deviceName;
    if (deviceBuild) systemBuild = deviceBuild;
    if (language) langRegion = language;
    
    systemVersion = osVersion;
    deviceType = devType;
    
    display_xres = xres;
    display_yres = yres;
    display_dpi = dpi;
    display_logical_dpi = dpi / 1.5f;  // 估算
    display_refresh_rate = refreshRate;
    
    INFO_LOG(Log::System, "OHOS System Properties Initialized:");
    INFO_LOG(Log::System, "  Device: %s (%s)", systemName.c_str(), systemBuild.c_str());
    INFO_LOG(Log::System, "  Language: %s", langRegion.c_str());
    INFO_LOG(Log::System, "  OS Version: %d, Device Type: %d", systemVersion, deviceType);
    INFO_LOG(Log::System, "  Display: %dx%d @%dDPI %.1fHz", display_xres, display_yres, display_dpi, display_refresh_rate);
}

void OhosSystemProperties_SetSafeInsets(float left, float top, float right, float bottom) {
    g_safeInsetLeft = left;
    g_safeInsetTop = top;
    g_safeInsetRight = right;
    g_safeInsetBottom = bottom;
    INFO_LOG(Log::System, "OHOS Safe Insets: L=%.1f T=%.1f R=%.1f B=%.1f", left, top, right, bottom);
}

void OhosSystemProperties_SetAudioConfig(int rate, int frames, int optRate, int optFrames) {
    sampleRate = rate;
    framesPerBuffer = frames;
    optimalSampleRate = optRate;
    optimalFramesPerBuffer = optFrames;
    INFO_LOG(Log::System, "OHOS Audio Config: %dHz %d frames (optimal: %dHz %d frames)",
             rate, frames, optRate, optFrames);
}

// ========== System_GetProperty 实现（参考 Android）==========
std::string System_GetProperty(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_NAME:
            return systemName;
        
        case SYSPROP_SYSTEMBUILD:
            return systemBuild;
        
        case SYSPROP_LANGREGION:
            return langRegion;
        
        case SYSPROP_CPUINFO:
            return cpuInfo;
        
        case SYSPROP_BOARDNAME:
            return boardName;
        
        case SYSPROP_GPUDRIVER_VERSION:
            return gpuDriverVersion;
        
        case SYSPROP_BUILD_VERSION:
            return PPSSPP_GIT_VERSION;
        
        case SYSPROP_COMPUTER_NAME:
            return systemName;
        
        case SYSPROP_CLIPBOARD_TEXT:
            return "";  // TODO: 实现剪贴板
        
        case SYSPROP_TEMP_DIRS:
            return OhosSystem::GetCacheDirectory();
        
        default:
            return "";
    }
}

// ========== System_GetPropertyStringVec 实现 ==========
std::vector<std::string> System_GetPropertyStringVec(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_ADDITIONAL_STORAGE_DIRS:
            return g_additionalStorageDirs;
        
        case SYSPROP_TEMP_DIRS:
        default:
            return {};
    }
}

// ========== System_GetPropertyInt 实现（参考 Android）==========
int64_t System_GetPropertyInt(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_SYSTEMVERSION:
            return systemVersion;
        
        case SYSPROP_DEVICE_TYPE:
            return deviceType;
        
        case SYSPROP_DISPLAY_XRES:
            return display_xres;
        
        case SYSPROP_DISPLAY_YRES:
            return display_yres;
        
        case SYSPROP_DISPLAY_DPI:
            return display_dpi;
        
        case SYSPROP_DISPLAY_LOGICAL_DPI:
            return display_logical_dpi;
        
        case SYSPROP_DISPLAY_COUNT:
            return display_count;
        
        case SYSPROP_MOGA_VERSION:
            return 0;
        
        case SYSPROP_AUDIO_SAMPLE_RATE:
            return sampleRate;
        
        case SYSPROP_AUDIO_FRAMES_PER_BUFFER:
            return framesPerBuffer;
        
        case SYSPROP_AUDIO_OPTIMAL_SAMPLE_RATE:
            return optimalSampleRate;
        
        case SYSPROP_AUDIO_OPTIMAL_FRAMES_PER_BUFFER:
            return optimalFramesPerBuffer;
        
        default:
            return -1;
    }
}

// ========== System_GetPropertyFloat 实现（参考 Android）==========
float System_GetPropertyFloat(SystemProperty prop) {
    switch (prop) {
        case SYSPROP_DISPLAY_REFRESH_RATE:
            return display_refresh_rate;
        
        case SYSPROP_DISPLAY_SAFE_INSET_LEFT:
            return g_safeInsetLeft * g_display.dpi_scale_x;
        
        case SYSPROP_DISPLAY_SAFE_INSET_RIGHT:
            return g_safeInsetRight * g_display.dpi_scale_x;
        
        case SYSPROP_DISPLAY_SAFE_INSET_TOP:
            return g_safeInsetTop * g_display.dpi_scale_y;
        
        case SYSPROP_DISPLAY_SAFE_INSET_BOTTOM:
            return g_safeInsetBottom * g_display.dpi_scale_y;
        
        default:
            return -1.0f;
    }
}


// ========== System_GetPropertyBool 实现（参考 Android）==========
bool System_GetPropertyBool(SystemProperty prop) {
    switch (prop) {
        // 文件浏览器相关
        case SYSPROP_HAS_FILE_BROWSER:
        case SYSPROP_HAS_FOLDER_BROWSER:
        case SYSPROP_HAS_IMAGE_BROWSER:
        case SYSPROP_HAS_OPEN_DIRECTORY:
        case SYSPROP_CAN_SHOW_FILE:
            return true;
        
        // 输入相关
        case SYSPROP_HAS_BACK_BUTTON:
        case SYSPROP_HAS_KEYBOARD:
        case SYSPROP_KEYBOARD_IS_SOFT:
        case SYSPROP_HAS_ACCELEROMETER:
            return true;
        
        // 剪贴板和文本
        case SYSPROP_HAS_TEXT_CLIPBOARD:
        case SYSPROP_HAS_TEXT_INPUT_DIALOG:
        case SYSPROP_SUPPORTS_SHARE_TEXT:
            return true;
        
        // 不支持的功能
        case SYSPROP_HAS_LOGIN_DIALOG:
        case SYSPROP_CAN_CREATE_SHORTCUT:
        case SYSPROP_SUPPORTS_HTTPS:
        case SYSPROP_DEBUGGER_PRESENT:
        case SYSPROP_HAS_ADDITIONAL_STORAGE:
        case SYSPROP_APP_GOLD:
        case SYSPROP_SUPPORTS_PERMISSIONS:
        case SYSPROP_SUPPORTS_SUSTAINED_PERF_MODE:
        case SYSPROP_ANDROID_SCOPED_STORAGE:
            return false;
        
        default:
            return false;
    }
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
