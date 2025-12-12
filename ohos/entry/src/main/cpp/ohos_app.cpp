/**
 * 鸿蒙应用层实现
 * 参考 android/jni/app-android.cpp
 */

#include "ohos_app.h"
#include "ohos_system.h"
#include "ohos_audio.h"
#include "ohos_input.h"
#include "ohos_vibration.h"
#include "ohos_hilog.h"  // 使用自定义的 hilog 包装器，避免 LogLevel 冲突
#include "Common/System/NativeApp.h"
#include "Common/GPU/Vulkan/VulkanLoader.h"  // for VulkanMayBeAvailable
#include "Core/Config.h"
#include <atomic>

#define APP_TAG "PPSSPP_App"

namespace OhosApp {

static std::atomic<bool> g_initialized(false);
static std::atomic<bool> g_running(false);
static std::atomic<bool> g_paused(false);

bool Initialize() {
    if (g_initialized.load()) {
        OHOS_LOGW(APP_TAG, "App already initialized");
        return true;
    }
    
    OHOS_LOGI(APP_TAG, "Initializing PPSSPP for HarmonyOS...");
    
    // 1. 检查系统接口是否已初始化（由 napi_init.cpp 的 InitEmulator 初始化）
    if (!OhosSystem::IsInitialized()) {
        OHOS_LOGE(APP_TAG, "System interface not initialized - call initEmulator() first");
        return false;
    }
    
    // 2. 初始化输入系统
    if (!OhosInput::Initialize()) {
        OHOS_LOGE(APP_TAG, "Failed to initialize input system");
        return false;
    }
    
    // 3. 初始化震动系统
    if (!OhosVibration::Initialize()) {
        OHOS_LOGE(APP_TAG, "Failed to initialize vibration system");
        return false;
    }
    
    // 4. 检测 Vulkan 可用性
    OHOS_LOGI(APP_TAG, "Checking Vulkan availability...");
    bool vulkanAvailable = VulkanMayBeAvailable();
    OHOS_LOGI(APP_TAG, "Vulkan available: %{public}s", vulkanAvailable ? "YES" : "NO");
    
    // 5. 初始化 PPSSPP Core
    // NativeInit 会初始化音频系统（通过 AudioBackend）
    OHOS_LOGI(APP_TAG, "Calling NativeInit...");
    
    // 获取目录路径
    std::string dataDir = OhosSystem::GetDataDirectory();
    std::string cacheDir = OhosSystem::GetCacheDirectory();
    
    OHOS_LOGI(APP_TAG, "Data dir: %{public}s", dataDir.c_str());
    OHOS_LOGI(APP_TAG, "Cache dir: %{public}s", cacheDir.c_str());
    
    // 准备参数
    const char *argv[] = {"ppsspp"};
    NativeInit(1, argv, dataDir.c_str(), dataDir.c_str(), cacheDir.c_str());
    
    OHOS_LOGI(APP_TAG, "NativeInit completed");
    
    // 注意：不要在这里强制设置 g_Config.bHapticFeedback = true
    // 这会覆盖用户保存的设置。PPSSPP 的配置系统会自动处理默认值和保存/加载。
    // 如果需要修改默认值，应该在 Core/Config.cpp 中修改 ConfigSetting 的定义。
    
    g_initialized.store(true);
    OHOS_LOGI(APP_TAG, "PPSSPP initialized successfully");
    
    return true;
}

void Shutdown() {
    if (!g_initialized.load()) {
        return;
    }
    
    OHOS_LOGI(APP_TAG, "Shutting down PPSSPP...");
    
    g_running.store(false);
    
    // 清理 PPSSPP Core（会清理音频系统）
    OHOS_LOGI(APP_TAG, "Calling NativeShutdown...");
    NativeShutdown();
    OHOS_LOGI(APP_TAG, "NativeShutdown completed");
    
    // 清理各个子系统
    OhosVibration::Shutdown();
    OhosInput::Shutdown();
    OhosSystem::Shutdown();
    
    g_initialized.store(false);
    OHOS_LOGI(APP_TAG, "PPSSPP shutdown complete");
}

bool LoadGame(const std::string& gamePath) {
    if (!g_initialized.load()) {
        OHOS_LOGE(APP_TAG, "Cannot load game: app not initialized");
        return false;
    }
    
    OHOS_LOGI(APP_TAG, "Loading game: %{public}s", gamePath.c_str());
    
    // TODO: 实现游戏加载
    // 1. 验证文件存在
    // 2. 检测游戏格式 (ISO/CSO/PBP)
    // 3. 加载游戏到 Core
    // 4. 初始化游戏状态
    
    g_running.store(true);
    g_paused.store(false);
    
    return true;
}

void RunFrame() {
    if (!g_running.load() || g_paused.load()) {
        return;
    }
    
    // TODO: 运行一帧
    // 1. 处理输入
    // 2. 更新模拟器状态
    // 3. 渲染画面
    // 4. 输出音频
}

void Pause() {
    OHOS_LOGI(APP_TAG, "Pausing emulator");
    g_paused.store(true);
    
    // 音频暂停由 PPSSPP Core 的 AudioBackend 管理
}

void Resume() {
    OHOS_LOGI(APP_TAG, "Resuming emulator");
    g_paused.store(false);
    
    // 音频恢复由 PPSSPP Core 的 AudioBackend 管理
}

bool IsRunning() {
    return g_running.load() && !g_paused.load();
}

} // namespace OhosApp
