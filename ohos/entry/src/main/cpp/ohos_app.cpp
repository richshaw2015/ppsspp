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
#include "Common/System/System.h"  // for SYSPROP_NAME, SYSPROP_SYSTEMVERSION
#include "Common/GPU/Vulkan/VulkanLoader.h"  // for VulkanMayBeAvailable
#include "Common/Log/LogManager.h"  // for LogManager
#include "Core/Config.h"
#include <atomic>
#include <dlfcn.h>  // for dlopen, dlclose, dlerror
#include <vulkan/vulkan.h>  // for Vulkan types and macros

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
    OHOS_LOGI(APP_TAG, "=== Checking Vulkan availability ===");
    OHOS_LOGI(APP_TAG, "Device name: %{public}s", System_GetProperty(SYSPROP_NAME).c_str());
    OHOS_LOGI(APP_TAG, "System version: %{public}d", (int)System_GetPropertyInt(SYSPROP_SYSTEMVERSION));
    
    // 启用 G3D 日志以查看 Vulkan 检测详情
    g_logManager.SetAllLogLevels(LogLevel::LDEBUG);
    OHOS_LOGI(APP_TAG, "Enabled debug logging for Vulkan detection");
    
    // 手动测试 Vulkan 库加载
    OHOS_LOGI(APP_TAG, "Attempting to load libvulkan.so...");
    void* vulkanLib = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (vulkanLib) {
        OHOS_LOGI(APP_TAG, "SUCCESS: libvulkan.so loaded at %{public}p", vulkanLib);
        
        // 尝试获取 vkEnumerateInstanceVersion 函数
        typedef VkResult (*PFN_vkEnumerateInstanceVersion)(uint32_t* pApiVersion);
        PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = 
            (PFN_vkEnumerateInstanceVersion)dlsym(vulkanLib, "vkEnumerateInstanceVersion");
        
        if (vkEnumerateInstanceVersion) {
            uint32_t apiVersion = 0;
            VkResult result = vkEnumerateInstanceVersion(&apiVersion);
            if (result == VK_SUCCESS) {
                uint32_t major = VK_VERSION_MAJOR(apiVersion);
                uint32_t minor = VK_VERSION_MINOR(apiVersion);
                uint32_t patch = VK_VERSION_PATCH(apiVersion);
                OHOS_LOGI(APP_TAG, "Vulkan API version: %{public}d.%{public}d.%{public}d", major, minor, patch);
            } else {
                OHOS_LOGW(APP_TAG, "vkEnumerateInstanceVersion failed: %{public}d", result);
            }
        } else {
            OHOS_LOGW(APP_TAG, "vkEnumerateInstanceVersion not found (Vulkan 1.0?)");
        }
        
        // 不要关闭库，让 VulkanMayBeAvailable 使用它
        // dlclose(vulkanLib);
    } else {
        OHOS_LOGE(APP_TAG, "FAILED to load libvulkan.so: %{public}s", dlerror());
    }
    
    // 手动执行 Vulkan 检测，获取详细信息
    OHOS_LOGI(APP_TAG, "=== Manual Vulkan Detection ===");
    
    bool manualVulkanAvailable = false;
    
    // 1. 加载 Vulkan 库（复用之前加载的库）
    void* manualVulkanLib = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!manualVulkanLib) {
        OHOS_LOGE(APP_TAG, "Failed to load libvulkan.so: %{public}s", dlerror());
    } else {
        OHOS_LOGI(APP_TAG, "Vulkan library loaded successfully");
        
        // 2. 获取必要的函数指针
        typedef VkResult (*PFN_vkEnumerateInstanceExtensionProperties)(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
        typedef VkResult (*PFN_vkCreateInstance)(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
        typedef VkResult (*PFN_vkEnumeratePhysicalDevices)(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
        typedef void (*PFN_vkDestroyInstance)(VkInstance instance, const VkAllocationCallbacks* pAllocator);
        typedef void (*PFN_vkGetPhysicalDeviceProperties)(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
        
        PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = 
            (PFN_vkEnumerateInstanceExtensionProperties)dlsym(manualVulkanLib, "vkEnumerateInstanceExtensionProperties");
        PFN_vkCreateInstance vkCreateInstance = 
            (PFN_vkCreateInstance)dlsym(manualVulkanLib, "vkCreateInstance");
        PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = 
            (PFN_vkEnumeratePhysicalDevices)dlsym(manualVulkanLib, "vkEnumeratePhysicalDevices");
        PFN_vkDestroyInstance vkDestroyInstance = 
            (PFN_vkDestroyInstance)dlsym(manualVulkanLib, "vkDestroyInstance");
        PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = 
            (PFN_vkGetPhysicalDeviceProperties)dlsym(manualVulkanLib, "vkGetPhysicalDeviceProperties");
        
        if (!vkEnumerateInstanceExtensionProperties || !vkCreateInstance || !vkEnumeratePhysicalDevices || !vkDestroyInstance || !vkGetPhysicalDeviceProperties) {
            OHOS_LOGE(APP_TAG, "Failed to get required Vulkan function pointers");
        } else {
            OHOS_LOGI(APP_TAG, "All required Vulkan functions loaded");
            
            // 3. 枚举实例扩展
            uint32_t extensionCount = 0;
            VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            if (result != VK_SUCCESS) {
                OHOS_LOGE(APP_TAG, "Failed to enumerate instance extensions: %{public}d", result);
            } else {
                OHOS_LOGI(APP_TAG, "Found %{public}d instance extensions", extensionCount);
                
                if (extensionCount > 0) {
                    std::vector<VkExtensionProperties> extensions(extensionCount);
                    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
                    if (result == VK_SUCCESS) {
                        bool foundSurface = false;
                        bool foundAndroidSurface = false;
                        
                        for (const auto& ext : extensions) {
                            OHOS_LOGI(APP_TAG, "Extension: %{public}s (v%{public}d)", ext.extensionName, ext.specVersion);
                            if (strcmp(ext.extensionName, "VK_KHR_surface") == 0) {
                                foundSurface = true;
                            }
                            if (strcmp(ext.extensionName, "VK_KHR_android_surface") == 0) {
                                foundAndroidSurface = true;
                            }
                        }
                        
                        if (!foundSurface) {
                            OHOS_LOGE(APP_TAG, "VK_KHR_surface extension not found");
                        } else if (!foundAndroidSurface) {
                            OHOS_LOGE(APP_TAG, "VK_KHR_android_surface extension not found");
                        } else {
                            OHOS_LOGI(APP_TAG, "Required surface extensions found");
                            
                            // 4. 创建 Vulkan 实例
                            VkApplicationInfo appInfo = {};
                            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                            appInfo.pApplicationName = "PPSSPP Vulkan Test";
                            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
                            appInfo.pEngineName = "PPSSPP Engine";
                            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
                            appInfo.apiVersion = VK_API_VERSION_1_0;
                            
                            const char* extensionNames[] = {
                                "VK_KHR_surface",
                                "VK_KHR_android_surface"
                            };
                            
                            VkInstanceCreateInfo createInfo = {};
                            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                            createInfo.pApplicationInfo = &appInfo;
                            createInfo.enabledExtensionCount = 2;
                            createInfo.ppEnabledExtensionNames = extensionNames;
                            
                            VkInstance instance;
                            result = vkCreateInstance(&createInfo, nullptr, &instance);
                            if (result != VK_SUCCESS) {
                                OHOS_LOGE(APP_TAG, "Failed to create Vulkan instance: %{public}d", result);
                            } else {
                                OHOS_LOGI(APP_TAG, "Vulkan instance created successfully");
                                
                                // 5. 枚举物理设备
                                uint32_t deviceCount = 0;
                                result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
                                if (result != VK_SUCCESS) {
                                    OHOS_LOGE(APP_TAG, "Failed to enumerate physical devices: %{public}d", result);
                                } else if (deviceCount == 0) {
                                    OHOS_LOGE(APP_TAG, "No Vulkan physical devices found");
                                } else {
                                    OHOS_LOGI(APP_TAG, "Found %{public}d Vulkan physical device(s)", deviceCount);
                                    
                                    std::vector<VkPhysicalDevice> devices(deviceCount);
                                    result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
                                    if (result == VK_SUCCESS) {
                                        for (uint32_t i = 0; i < deviceCount; i++) {
                                            VkPhysicalDeviceProperties props;
                                            vkGetPhysicalDeviceProperties(devices[i], &props);
                                            OHOS_LOGI(APP_TAG, "Device %{public}d: %{public}s (Type: %{public}d)", 
                                                     i, props.deviceName, props.deviceType);
                                            
                                            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
                                                props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ||
                                                props.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
                                                manualVulkanAvailable = true;
                                                OHOS_LOGI(APP_TAG, "Found suitable GPU: %{public}s", props.deviceName);
                                            }
                                        }
                                    }
                                }
                                
                                vkDestroyInstance(instance, nullptr);
                            }
                        }
                    }
                }
            }
        }
        
        // 不要关闭库，让 VulkanMayBeAvailable 使用它
        // dlclose(vulkanLib);
    }
    
    OHOS_LOGI(APP_TAG, "=== Manual Vulkan Detection Complete ===");
    OHOS_LOGI(APP_TAG, "Manual Vulkan detection result: %{public}s", manualVulkanAvailable ? "AVAILABLE" : "NOT AVAILABLE");
    
    // 6. 调用原始的 VulkanMayBeAvailable 进行对比
    OHOS_LOGI(APP_TAG, "Checking Vulkan support via VulkanMayBeAvailable()...");
    bool vulkanAvailable = VulkanMayBeAvailable();
    OHOS_LOGI(APP_TAG, "VulkanMayBeAvailable() result: %{public}s", vulkanAvailable ? "YES" : "NO");
    
    if (!vulkanAvailable) {
        OHOS_LOGW(APP_TAG, "VulkanMayBeAvailable() returned NO, but manual detection says: %{public}s", 
                  manualVulkanAvailable ? "YES" : "NO");
    }
    
    OHOS_LOGI(APP_TAG, "=== Vulkan check complete ===");
    
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
