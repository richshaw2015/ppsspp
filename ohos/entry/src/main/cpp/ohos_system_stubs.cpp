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

void System_Notify(SystemNotification notification) {
    // 空实现 - 系统通知
    INFO_LOG(Log::System, "System_Notify: %d", (int)notification);
}

// ============================================================================
// System 启动和交互函数
// ============================================================================

void System_LaunchUrl(LaunchUrlType urlType, std::string_view url) {
    // 启动 URL - 空实现
    // TODO: 实现鸿蒙浏览器启动
    INFO_LOG(Log::System, "System_LaunchUrl: type=%d, url=%.*s", (int)urlType, (int)url.size(), url.data());
}

std::vector<std::string> System_GetPropertyStringVec(SystemProperty prop) {
    // 获取字符串向量属性 - 返回空向量
    // TODO: 实现存储路径列表等
    INFO_LOG(Log::System, "System_GetPropertyStringVec: %d (returning empty)", (int)prop);
    return std::vector<std::string>();
}

void System_Toast(std::string_view text) {
    // 显示 Toast - 空实现
    // TODO: 实现鸿蒙 Toast
    INFO_LOG(Log::System, "System_Toast: %.*s", (int)text.size(), text.data());
}

void System_Vibrate(int length_ms) {
    INFO_LOG(Log::System, "System_Vibrate: %d ms", length_ms);
    OhosVibration::Vibrate(length_ms);
}

// ============================================================================
// 权限系统函数
// ============================================================================

PermissionStatus System_GetPermissionStatus(SystemPermission permission) {
    // 鸿蒙权限系统 - 暂时返回已授权
    // TODO: 实现真正的权限检查
    INFO_LOG(Log::System, "System_GetPermissionStatus: %d (returning GRANTED)", (int)permission);
    return PERMISSION_STATUS_GRANTED;
}

void System_AskForPermission(SystemPermission permission) {
    // 鸿蒙权限请求 - 空实现
    // TODO: 实现权限请求对话框
    INFO_LOG(Log::System, "System_AskForPermission: %d", (int)permission);
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
// ImGui 平台函数（ImGui 库已链接，只需平台层桩）
// ============================================================================

// ImGui 平台初始化
void ImGui_ImplPlatform_Init(const Path &fontPath) {
    // ImGui 平台初始化 - 空实现
    // TODO: 实现 ImGui 平台层
    WARN_LOG(Log::System, "ImGui_ImplPlatform_Init: not implemented");
}

// ImGui 平台新帧
void ImGui_ImplPlatform_NewFrame() {
    // ImGui 平台新帧 - 空实现
}

// ImGui 平台按键事件
bool ImGui_ImplPlatform_KeyEvent(const KeyInput &key) {
    // ImGui 平台按键事件 - 空实现
    return false;
}

// ImGui 平台触摸事件
bool ImGui_ImplPlatform_TouchEvent(const TouchInput &touch) {
    // ImGui 平台触摸事件 - 空实现
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
