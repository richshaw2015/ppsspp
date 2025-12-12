// HarmonyOS 系统功能桩实现
// 提供 PPSSPP 需要的各种系统接口的空实现
// 
// 注意：以下函数已在 UI 库中实现，不需要在这里重复：
// - System_PostUIMessage (UI/NativeApp.cpp)
// - System_RunOnMainThread (UI/NativeApp.cpp)
// - NativeSaveSecret (UI/NativeApp.cpp)
// - NativeLoadSecret (UI/NativeApp.cpp)
// - System_AudioGetDebugStats (UI/AudioCommon.cpp)
// - System_AudioClear (UI/AudioCommon.cpp)
// - System_AudioPushSamples (UI/AudioCommon.cpp)

#include "Common/System/System.h"
#include "Common/System/NativeApp.h"
#include "Common/Input/InputState.h"
#include "Common/Log.h"
#include "Common/Math/lin/matrix4x4.h"
#include "Common/VR/PPSSPPVR.h"
#include "Core/System.h"
#include "Core/Config.h"
#include "ohos_vibration.h"
#include "ohos_hilog.h"
#include <string>
#include <vector>

// ============================================================================
// VR 相关函数
// ============================================================================

bool IsGameVRScene() {
    return false;
}

bool IsPassthroughSupported() {
    return false;
}

// ============================================================================
// System 属性函数
// ============================================================================

int64_t System_GetPropertyInt(SystemProperty prop) {
    std::string value = System_GetProperty(prop);
    if (value.empty()) {
        return 0;
    }
    return std::stoll(value);
}

// ============================================================================
// System 通知函数
// ============================================================================

// 前向声明 - 在 napi_ppsspp.cpp 中实现
namespace NapiPPSSPP {
    bool RecreateActivity();
    bool SetImmersiveMode(bool immersive);
    bool UpdateScreenRotation();
}

// 前向声明 - 在 ohos_audio_backend.cpp 中实现
void OhosAudio_ResetDevice();

void System_Notify(SystemNotification notification) {
    switch (notification) {
        case SystemNotification::ROTATE_UPDATED:
            // 屏幕旋转更新
            INFO_LOG(Log::System, "System_Notify: ROTATE_UPDATED");
            NapiPPSSPP::UpdateScreenRotation();
            break;
        
        case SystemNotification::FORCE_RECREATE_ACTIVITY:
            // 强制重建 Activity（用于显示设置变更）
            INFO_LOG(Log::System, "System_Notify: FORCE_RECREATE_ACTIVITY");
            NapiPPSSPP::RecreateActivity();
            break;
        
        case SystemNotification::IMMERSIVE_MODE_CHANGE:
            // 沉浸模式变更
            INFO_LOG(Log::System, "System_Notify: IMMERSIVE_MODE_CHANGE");
            NapiPPSSPP::SetImmersiveMode(true);
            break;
        
        case SystemNotification::AUDIO_RESET_DEVICE:
            // 音频设备重置
            INFO_LOG(Log::System, "System_Notify: AUDIO_RESET_DEVICE");
            OhosAudio_ResetDevice();
            break;
        
        case SystemNotification::AUDIO_MODE_CHANGED:
            // 音频模式变更（静音模式、混音模式等）
            INFO_LOG(Log::System, "System_Notify: AUDIO_MODE_CHANGED");
            // OHOS 上音频模式由系统管理，不需要特殊处理
            break;
        
        case SystemNotification::KEEP_SCREEN_AWAKE:
            // 保持屏幕唤醒（游戏运行时）
            // 注意：这个通知在每帧渲染时都会发送，不需要每次都调用 ArkTS
            // 实际的屏幕常亮控制通过 SET_KEEP_SCREEN_BRIGHT 请求处理
            break;
        
        case SystemNotification::ACTIVITY:
            // 用户活动（触摸、按键等）- 用于屏幕保护程序
            // OHOS 系统会自动处理，不需要特殊处理
            break;
        
        case SystemNotification::UI_STATE_CHANGED:
            // UI 状态变更
            INFO_LOG(Log::System, "System_Notify: UI_STATE_CHANGED");
            break;
        
        case SystemNotification::BOOT_DONE:
            // 游戏启动完成
            INFO_LOG(Log::System, "System_Notify: BOOT_DONE");
            break;
        
        case SystemNotification::DEBUG_MODE_CHANGE:
        case SystemNotification::DISASSEMBLY:
        case SystemNotification::DISASSEMBLY_AFTERSTEP:
        case SystemNotification::MEM_VIEW:
        case SystemNotification::SYMBOL_MAP_UPDATED:
            // 调试相关通知 - 在 OHOS 上不需要处理
            break;
        
        default:
            INFO_LOG(Log::System, "System_Notify: unhandled notification %d", (int)notification);
            break;
    }
}

// ============================================================================
// System 键盘函数
// ============================================================================

// 前向声明 - 在 napi_ppsspp.cpp 中实现
namespace NapiPPSSPP {
    bool ShowKeyboard();
}

void System_ShowKeyboard() {
    INFO_LOG(Log::System, "System_ShowKeyboard");
    NapiPPSSPP::ShowKeyboard();
}

// ============================================================================
// System 启动和交互函数
// ============================================================================

// 前向声明 - 在 napi_ppsspp.cpp 中实现
namespace NapiPPSSPP {
    bool OpenUrl(const std::string& url);
}

void System_LaunchUrl(LaunchUrlType urlType, std::string_view url) {
    INFO_LOG(Log::System, "System_LaunchUrl: type=%d, url=%.*s", (int)urlType, (int)url.size(), url.data());
    
    // 调用 NAPI 层打开 URL
    std::string urlStr(url);
    bool success = NapiPPSSPP::OpenUrl(urlStr);
    
    if (!success) {
        WARN_LOG(Log::System, "System_LaunchUrl: failed to open URL");
    }
}

std::vector<std::string> System_GetPropertyStringVec(SystemProperty prop) {
    // 获取字符串向量属性 - 返回空向量
    // TODO: 实现存储路径列表等
    INFO_LOG(Log::System, "System_GetPropertyStringVec: %d (returning empty)", (int)prop);
    return std::vector<std::string>();
}

// 前向声明 - 在 napi_ppsspp.cpp 中实现
namespace NapiPPSSPP {
    bool ShowToast(const std::string& message);
}

void System_Toast(std::string_view text) {
    INFO_LOG(Log::System, "System_Toast: %.*s", (int)text.size(), text.data());
    
    // 调用 NAPI 层显示 Toast
    std::string message(text);
    NapiPPSSPP::ShowToast(message);
}

void System_Vibrate(int length_ms) {
    INFO_LOG(Log::System, "System_Vibrate: %d ms", length_ms);
    OhosVibration::Vibrate(length_ms);
}

// ============================================================================
// 权限系统函数
// ============================================================================

// OHOS 权限说明：
// 1. OHOS 使用沙箱机制，应用可以自由访问自己的沙箱目录，不需要额外权限
// 2. 文件选择器（DocumentViewPicker）会自动处理文件访问权限
// 3. 需要在 module.json5 中声明的权限（如 INTERNET、VIBRATE）在安装时自动授予
// 4. 因此，对于 PPSSPP 的主要用例，存储权限始终返回 GRANTED 是合理的
//
// 如果将来需要访问用户媒体文件，需要：
// 1. 在 module.json5 中声明 ohos.permission.READ_MEDIA 等权限
// 2. 使用 abilityAccessCtrl API 检查和请求权限

PermissionStatus System_GetPermissionStatus(SystemPermission permission) {
    switch (permission) {
        case SYSTEM_PERMISSION_STORAGE:
            // OHOS 沙箱机制：应用沙箱内的文件不需要权限
            // 文件选择器会自动处理外部文件的访问权限
            INFO_LOG(Log::System, "System_GetPermissionStatus: STORAGE -> GRANTED (OHOS sandbox)");
            return PERMISSION_STATUS_GRANTED;
        
        default:
            INFO_LOG(Log::System, "System_GetPermissionStatus: %d -> GRANTED (default)", (int)permission);
            return PERMISSION_STATUS_GRANTED;
    }
}

void System_AskForPermission(SystemPermission permission) {
    // OHOS 权限请求
    // 由于 PPSSPP 使用的权限（存储、网络、震动）都是在安装时授予的，
    // 或者通过文件选择器自动处理，因此这里不需要实际的权限请求对话框
    
    switch (permission) {
        case SYSTEM_PERMISSION_STORAGE:
            INFO_LOG(Log::System, "System_AskForPermission: STORAGE (no action needed on OHOS)");
            // 在 OHOS 上，存储权限通过沙箱机制和文件选择器自动处理
            // 直接发送权限已授予的消息
            System_PostUIMessage(UIMessage::PERMISSION_GRANTED, "storage");
            break;
        
        default:
            INFO_LOG(Log::System, "System_AskForPermission: %d (no action)", (int)permission);
            break;
    }
}

// ============================================================================
// 音频后端函数
// ============================================================================

#include "ohos_audio_backend.h"

AudioBackend *System_CreateAudioBackend() {
    // 创建鸿蒙音频后端
    INFO_LOG(Log::System, "System_CreateAudioBackend: creating OhosAudioBackend");
    return new OhosAudioBackend();
}

// ============================================================================
// VR 输入函数
// ============================================================================

bool UpdateVRKeys(const KeyInput &key) {
    // VR 按键更新 - 空实现（鸿蒙不支持 VR）
    return false;
}

bool UpdateVRAxis(const AxisInput *axes, size_t count) {
    // VR 轴输入更新 - 空实现（鸿蒙不支持 VR）
    return false;
}

void SetVRAppMode(VRAppMode mode) {
    // 设置 VR 应用模式 - 空实现（鸿蒙不支持 VR）
}

// ============================================================================
// ImGui 平台函数
// 注意：ImGui 平台层实现在 ext/imgui/imgui_impl_platform.cpp
// 这里不需要重复实现，CMakeLists.txt 已经链接了该文件
// ============================================================================

// 以下函数已在 ext/imgui/imgui_impl_platform.cpp 中实现：
// - ImGui_ImplPlatform_Init
// - ImGui_ImplPlatform_NewFrame
// - ImGui_ImplPlatform_KeyEvent (通过 ImGui_ImplPlatform_KeyEvent)
// - ImGui_ImplPlatform_TouchEvent (通过 ImGui_ImplPlatform_TouchEvent)

// ============================================================================
// 摄像头和音频录制函数
// ============================================================================

std::vector<std::string> System_GetCameraDeviceList() {
    // OHOS 摄像头设备列表 - 暂不实现
    INFO_LOG(Log::System, "System_GetCameraDeviceList: returning empty list");
    return std::vector<std::string>();
}

bool System_AudioRecordingIsAvailable() {
    // OHOS 音频录制 - 暂不实现
    INFO_LOG(Log::System, "System_AudioRecordingIsAvailable: returning false");
    return false;
}

bool System_AudioRecordingState() {
    // OHOS 音频录制状态 - 暂不实现
    return false;
}

// ============================================================================
// CityHash 函数（用于哈希计算）
// ============================================================================

// CityHash64 是 C 函数，不需要 extern "C"（已经在头文件中声明）
uint64_t CityHash64(const char *buf, size_t len) {
    // 简单的哈希实现（不是真正的 CityHash，但足够用于调试）
    uint64_t hash = 0;
    for (size_t i = 0; i < len; i++) {
        hash = hash * 31 + (unsigned char)buf[i];
    }
    return hash;
}

// ============================================================================
// 编译器和平台信息
// ============================================================================

const char *GetCompilerABI() {
    return "clang-ohos-aarch64";
}

// ============================================================================
// 全局变量（Android 兼容）
// ============================================================================

// Android 外部存储路径
std::string g_extFilesDir = "";
std::string g_externalDir = "";
