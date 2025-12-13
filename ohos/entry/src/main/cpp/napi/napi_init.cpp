/**
 * NAPI 模块初始化
 * 负责注册所有导出到 ArkTS 的函数
 * 
 * 参考 Android 实现：android/jni/app-android.cpp
 */

#include <napi/native_api.h>
#include <ace/xcomponent/native_interface_xcomponent.h>
#include <rawfile/raw_file_manager.h>
#include <cstring>  // for memset
#include "../ohos_hilog.h"  // 使用自定义的 hilog 包装器
#include "../ohos_xcomponent.h"
#include "../ohos_system.h"
#include "../ohos_app.h"  // for OhosApp::Initialize
#include "../ohos_rawfile_reader.h"
#include "napi_ppsspp.h"
#include "Common/Log.h"
#include "Common/System/NativeApp.h"
#include "Common/System/System.h"
#include "Common/System/Display.h"
#include "Common/File/VFS/VFS.h"
#include "Core/Config.h"
#include "Core/ConfigValues.h"
#include "Core/System.h"

#define NAPI_TAG "PPSSPP_NAPI"

// 全局变量
static OH_NativeXComponent* g_nativeXComponent = nullptr;
static bool g_nativeInitCalled = false;
static NativeResourceManager* g_resourceManager = nullptr;

// XComponent 回调函数
__attribute__((noinline))
static void OnSurfaceCreatedCB(OH_NativeXComponent* component, void* window) {
    // 使用 volatile 防止编译器优化掉日志调用
    volatile int dummy = 1;
    (void)dummy;
    
    OHOS_LOGI(NAPI_TAG, "########## OnSurfaceCreatedCB CALLED ##########");
    OHOS_LOGI(NAPI_TAG, "component=%{public}p, window=%{public}p", (void*)component, window);
    
    if (component == nullptr) {
        OHOS_LOGE(NAPI_TAG, "component is NULL in callback!");
        return;
    }
    if (window == nullptr) {
        OHOS_LOGE(NAPI_TAG, "window is NULL in callback!");
        return;
    }
    
    OhosXComponent::OnSurfaceCreated(component, window);
    OHOS_LOGI(NAPI_TAG, "########## OnSurfaceCreatedCB DONE ##########");
}

static void OnSurfaceChangedCB(OH_NativeXComponent* component, void* window) {
    OHOS_LOGI(NAPI_TAG, "########## OnSurfaceChangedCB CALLED ##########");
    OhosXComponent::OnSurfaceChanged(component, window);
}

static void OnSurfaceDestroyedCB(OH_NativeXComponent* component, void* window) {
    OHOS_LOGI(NAPI_TAG, "########## OnSurfaceDestroyedCB CALLED ##########");
    OhosXComponent::OnSurfaceDestroyed(component, window);
}

static void DispatchTouchEventCB(OH_NativeXComponent* component, void* window) {
    OhosXComponent::OnTouchEvent(component, window);
}

// 静态回调结构体 - 必须是静态的，因为 XComponent 会持有指向它的指针
static OH_NativeXComponent_Callback g_xcomponentCallback;
static bool g_callbackInitialized = false;

static void InitCallbackStruct() {
    if (!g_callbackInitialized) {
        memset(&g_xcomponentCallback, 0, sizeof(g_xcomponentCallback));
        g_xcomponentCallback.OnSurfaceCreated = OnSurfaceCreatedCB;
        g_xcomponentCallback.OnSurfaceChanged = OnSurfaceChangedCB;
        g_xcomponentCallback.OnSurfaceDestroyed = OnSurfaceDestroyedCB;
        g_xcomponentCallback.DispatchTouchEvent = DispatchTouchEventCB;
        g_callbackInitialized = true;
        OHOS_LOGI(NAPI_TAG, "Callback struct initialized");
    }
}

/**
 * 初始化 XComponent 的通用函数
 * 避免重复代码
 */
static bool InitializeXComponent(OH_NativeXComponent* component, const char* source) {
    OHOS_LOGI(NAPI_TAG, "InitializeXComponent from %{public}s", source);
    
    if (!component) {
        OHOS_LOGE(NAPI_TAG, "component is null!");
        return false;
    }
    
    // 防止重复初始化
    if (g_nativeXComponent != nullptr) {
        OHOS_LOGW(NAPI_TAG, "XComponent already initialized, skipping");
        return true;
    }
    
    // 获取 XComponent ID
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, idStr, &idSize) == OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        OHOS_LOGI(NAPI_TAG, "XComponent ID: %{public}s", idStr);
    }
    
    // 保存 XComponent 引用
    g_nativeXComponent = component;
    
    // 初始化回调结构体
    InitCallbackStruct();
    
    // 注册回调 - 使用静态结构体，确保生命周期
    OHOS_LOGI(NAPI_TAG, "Registering XComponent callbacks...");
    OHOS_LOGI(NAPI_TAG, "Callback addresses: OnSurfaceCreated=%{public}p, OnSurfaceChanged=%{public}p, OnSurfaceDestroyed=%{public}p, DispatchTouchEvent=%{public}p",
              (void*)g_xcomponentCallback.OnSurfaceCreated,
              (void*)g_xcomponentCallback.OnSurfaceChanged,
              (void*)g_xcomponentCallback.OnSurfaceDestroyed,
              (void*)g_xcomponentCallback.DispatchTouchEvent);
    
    int32_t ret = OH_NativeXComponent_RegisterCallback(component, &g_xcomponentCallback);
    OHOS_LOGI(NAPI_TAG, "OH_NativeXComponent_RegisterCallback returned: %{public}d", ret);
    if (ret == OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        OHOS_LOGI(NAPI_TAG, "Callbacks registered successfully");
    } else {
        OHOS_LOGE(NAPI_TAG, "Failed to register callbacks: %{public}d", ret);
        return false;
    }
    
    // 根据配置选择图形后端
    // 注意：此时 g_Config 可能还未加载，所以先使用 OpenGL
    // 实际的后端切换会在用户更改设置后通过重启应用来实现
    OhosXComponent::GraphicsBackend backend = OhosXComponent::GraphicsBackend::OPENGL;
    
    // 如果配置已加载，检查用户选择的后端
    if (g_Config.iGPUBackend == (int)GPUBackend::VULKAN) {
        OHOS_LOGI(NAPI_TAG, "User selected Vulkan backend");
        backend = OhosXComponent::GraphicsBackend::VULKAN;
    } else {
        OHOS_LOGI(NAPI_TAG, "Using OpenGL backend (config: %{public}d)", g_Config.iGPUBackend);
    }
    
    // 初始化 XComponent
    OHOS_LOGI(NAPI_TAG, "Initializing XComponent with backend: %{public}s", 
              backend == OhosXComponent::GraphicsBackend::VULKAN ? "Vulkan" : "OpenGL");
    bool success = OhosXComponent::Initialize(component, backend);
    if (success) {
        OHOS_LOGI(NAPI_TAG, "XComponent initialized successfully");
    } else {
        OHOS_LOGE(NAPI_TAG, "Failed to initialize XComponent");
    }
    
    return success;
}

/**
 * NativeXComponentInit - 由 XComponent 的 libraryname 参数触发（旧 API）
 * 注意：从 API 10 开始，这个函数可能不会被调用
 */
extern "C" __attribute__((visibility("default")))
void NativeXComponentInit(OH_NativeXComponent* component) {
    OHOS_LOGI(NAPI_TAG, "========== NativeXComponentInit ==========");
    InitializeXComponent(component, "NativeXComponentInit");
}

/**
 * 初始化模拟器 - 调用 NativeInit
 * 参数: filesDir (string), cacheDir (string)
 */
static napi_value InitEmulator(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_TAG, "========== InitEmulator START ==========");
    
    if (g_nativeInitCalled) {
        OHOS_LOGW(NAPI_TAG, "NativeInit already called");
        napi_value result;
        napi_get_boolean(env, true, &result);
        return result;
    }
    
    // 获取参数 - 必须从 ArkTS 层传入 context.filesDir, context.cacheDir 和 context.resourceManager
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    // 验证参数数量
    if (argc < 2) {
        OHOS_LOGE(NAPI_TAG, "initEmulator requires at least 2 arguments: filesDir and cacheDir");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 获取 filesDir 参数 (context.filesDir)
    std::string dataDir;
    {
        size_t strSize = 0;
        napi_get_value_string_utf8(env, args[0], nullptr, 0, &strSize);
        if (strSize == 0) {
            OHOS_LOGE(NAPI_TAG, "filesDir is empty");
            napi_value result;
            napi_get_boolean(env, false, &result);
            return result;
        }
        dataDir.resize(strSize);
        napi_get_value_string_utf8(env, args[0], &dataDir[0], strSize + 1, &strSize);
    }
    
    // 获取 cacheDir 参数 (context.cacheDir)
    std::string cacheDir;
    {
        size_t strSize = 0;
        napi_get_value_string_utf8(env, args[1], nullptr, 0, &strSize);
        if (strSize == 0) {
            OHOS_LOGE(NAPI_TAG, "cacheDir is empty");
            napi_value result;
            napi_get_boolean(env, false, &result);
            return result;
        }
        cacheDir.resize(strSize);
        napi_get_value_string_utf8(env, args[1], &cacheDir[0], strSize + 1, &strSize);
    }
    
    OHOS_LOGI(NAPI_TAG, "Data dir: %{public}s", dataDir.c_str());
    OHOS_LOGI(NAPI_TAG, "Cache dir: %{public}s", cacheDir.c_str());
    
    // 初始化 OhosSystem - 保存路径供其他模块使用
    if (!OhosSystem::Initialize(dataDir, cacheDir)) {
        OHOS_LOGE(NAPI_TAG, "Failed to initialize OhosSystem");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 获取 ResourceManager（如果提供）
    if (argc >= 3) {
        g_resourceManager = OH_ResourceManager_InitNativeResourceManager(env, args[2]);
        if (g_resourceManager) {
            OHOS_LOGI(NAPI_TAG, "ResourceManager initialized successfully");
            
            // 注册 Rawfile Reader 到 VFS
            g_VFS.Register("", new OhosRawfileReader(g_resourceManager));
            OHOS_LOGI(NAPI_TAG, "Rawfile Reader registered to VFS");
        } else {
            OHOS_LOGW(NAPI_TAG, "Failed to initialize ResourceManager");
        }
    } else {
        OHOS_LOGW(NAPI_TAG, "ResourceManager not provided, will use file system only");
    }
    
    // 调用 NativeInit - 这是 PPSSPP 的核心初始化函数
    const char* argv[] = { "PPSSPP" };
    OHOS_LOGI(NAPI_TAG, "Calling NativeInit...");
    NativeInit(1, argv, dataDir.c_str(), dataDir.c_str(), cacheDir.c_str());
    
    g_nativeInitCalled = true;
    OHOS_LOGI(NAPI_TAG, "NativeInit completed");
    
    // 调用 OhosApp::Initialize() 进行额外的初始化（包括 Vulkan 检测）
    OHOS_LOGI(NAPI_TAG, "Calling OhosApp::Initialize...");
    bool appInitSuccess = OhosApp::Initialize();
    OHOS_LOGI(NAPI_TAG, "OhosApp::Initialize %{public}s", appInitSuccess ? "succeeded" : "failed");
    
    OHOS_LOGI(NAPI_TAG, "========== InitEmulator END ==========");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

/**
 * 关闭模拟器
 */
static napi_value ShutdownEmulator(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_TAG, "ShutdownEmulator called");
    
    // 停止渲染循环
    OhosXComponent::StopRenderLoop();
    
    // 调用 NativeShutdown
    if (g_nativeInitCalled) {
        NativeShutdown();
        g_nativeInitCalled = false;
    }
    
    return nullptr;
}

/**
 * 加载游戏
 */
static napi_value LoadGame(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OHOS_LOGE(NAPI_TAG, "LoadGame: missing path argument");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 获取游戏路径
    size_t strSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &strSize);
    char* gamePath = new char[strSize + 1];
    napi_get_value_string_utf8(env, args[0], gamePath, strSize + 1, &strSize);
    
    OHOS_LOGI(NAPI_TAG, "Loading game: %{public}s", gamePath);
    
    // TODO: 实现游戏加载
    
    delete[] gamePath;
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

/**
 * 初始化系统属性（参考 Android 实现）
 * 参数：deviceName, deviceBuild, language, osVersion, devType, xres, yres, dpi, refreshRate
 */
static napi_value InitSystemProperties(napi_env env, napi_callback_info info) {
    size_t argc = 9;
    napi_value args[9];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 9) {
        OHOS_LOGE(NAPI_TAG, "InitSystemProperties requires 9 arguments");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 获取字符串参数
    char deviceName[256] = {0}, deviceBuild[256] = {0}, language[32] = {0};
    size_t len;
    napi_get_value_string_utf8(env, args[0], deviceName, sizeof(deviceName), &len);
    napi_get_value_string_utf8(env, args[1], deviceBuild, sizeof(deviceBuild), &len);
    napi_get_value_string_utf8(env, args[2], language, sizeof(language), &len);
    
    // 获取整数参数
    int32_t osVersion, devType, xres, yres, dpi;
    napi_get_value_int32(env, args[3], &osVersion);
    napi_get_value_int32(env, args[4], &devType);
    napi_get_value_int32(env, args[5], &xres);
    napi_get_value_int32(env, args[6], &yres);
    napi_get_value_int32(env, args[7], &dpi);
    
    // 获取浮点参数
    double refreshRate;
    napi_get_value_double(env, args[8], &refreshRate);
    
    OHOS_LOGI(NAPI_TAG, "Initializing system properties:");
    OHOS_LOGI(NAPI_TAG, "  Device: %{public}s (%{public}s)", deviceName, deviceBuild);
    OHOS_LOGI(NAPI_TAG, "  Language: %{public}s, OS Version: %{public}d, Device Type: %{public}d", language, osVersion, devType);
    OHOS_LOGI(NAPI_TAG, "  Display: %{public}dx%{public}d @%{public}dDPI %.1fHz", xres, yres, dpi, refreshRate);
    
    // 调用 C++ 初始化函数
    OhosSystemProperties_Init(
        deviceName, deviceBuild, language,
        osVersion, devType,
        xres, yres, dpi, (float)refreshRate
    );
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

/**
 * 设置安全区域 insets
 * 参数：left, top, right, bottom (number)
 */
static napi_value SetSafeInsets(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 4) {
        OHOS_LOGE(NAPI_TAG, "SetSafeInsets requires 4 arguments");
        return nullptr;
    }
    
    // 获取参数
    double left, top, right, bottom;
    napi_get_value_double(env, args[0], &left);
    napi_get_value_double(env, args[1], &top);
    napi_get_value_double(env, args[2], &right);
    napi_get_value_double(env, args[3], &bottom);
    
    OHOS_LOGI(NAPI_TAG, "Setting safe insets: left=%{public}.1f, top=%{public}.1f, right=%{public}.1f, bottom=%{public}.1f",
              left, top, right, bottom);
    
    // 设置安全区域（同时调用两个函数）
    OhosSystem::SetSafeInsets((float)left, (float)top, (float)right, (float)bottom);
    OhosSystemProperties_SetSafeInsets((float)left, (float)top, (float)right, (float)bottom);
    
    return nullptr;
}

/**
 * 设置音频配置
 * 参数：sampleRate, framesPerBuffer, optimalSampleRate, optimalFramesPerBuffer
 */
static napi_value SetAudioConfig(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 4) {
        OHOS_LOGE(NAPI_TAG, "SetAudioConfig requires 4 arguments");
        return nullptr;
    }
    
    // 获取参数
    int32_t rate, frames, optRate, optFrames;
    napi_get_value_int32(env, args[0], &rate);
    napi_get_value_int32(env, args[1], &frames);
    napi_get_value_int32(env, args[2], &optRate);
    napi_get_value_int32(env, args[3], &optFrames);
    
    OHOS_LOGI(NAPI_TAG, "Setting audio config: %{public}dHz %{public}d frames (optimal: %{public}dHz %{public}d frames)",
              rate, frames, optRate, optFrames);
    
    // 调用 C++ 设置函数
    OhosSystemProperties_SetAudioConfig(rate, frames, optRate, optFrames);
    
    return nullptr;
}

/**
 * 发送触摸事件
 */
static napi_value SendTouchEvent(napi_env env, napi_callback_info info) {
    return nullptr;
}

// SendKeyEvent 已移至 NapiPPSSPP::SendKeyEvent

/**
 * 从 exports 中获取 XComponent 并注册回调
 * 根据官方文档，当使用 libraryname 时，XComponent 会通过 exports 传递
 */
static void TryGetXComponentFromExports(napi_env env, napi_value exports) {
    OHOS_LOGI(NAPI_TAG, "Trying to get XComponent from exports...");
    
    // 检查是否已经初始化
    if (g_nativeXComponent != nullptr) {
        OHOS_LOGW(NAPI_TAG, "XComponent already initialized, skipping");
        return;
    }
    
    // 尝试获取 __NATIVE_XCOMPONENT_OBJ__ 属性
    napi_value exportInstance = nullptr;
    napi_status status = napi_get_named_property(env, exports, OH_NATIVE_XCOMPONENT_OBJ, &exportInstance);
    
    if (status != napi_ok) {
        OHOS_LOGW(NAPI_TAG, "napi_get_named_property failed: %{public}d", status);
        return;
    }
    
    // 检查值类型
    napi_valuetype valueType;
    napi_typeof(env, exportInstance, &valueType);
    OHOS_LOGI(NAPI_TAG, "exportInstance type: %{public}d", valueType);
    
    if (valueType == napi_undefined || valueType == napi_null) {
        OHOS_LOGW(NAPI_TAG, "XComponent not found in exports (undefined/null)");
        return;
    }
    
    OHOS_LOGI(NAPI_TAG, "Found XComponent object in exports!");
    
    // 获取 native XComponent 指针
    OH_NativeXComponent* nativeXComponent = nullptr;
    status = napi_unwrap(env, exportInstance, reinterpret_cast<void**>(&nativeXComponent));
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_TAG, "napi_unwrap failed: %{public}d", status);
        return;
    }
    
    if (nativeXComponent == nullptr) {
        OHOS_LOGE(NAPI_TAG, "nativeXComponent is null after unwrap");
        return;
    }
    
    OHOS_LOGI(NAPI_TAG, "Got native XComponent pointer: %{public}p", nativeXComponent);
    
    // 使用通用初始化函数
    InitializeXComponent(nativeXComponent, "TryGetXComponentFromExports");
    
    // 注意：OnSurfaceCreated 回调会在 surface 创建时由系统自动调用
    // 我们不需要手动触发，只需要确保回调已经注册
    OHOS_LOGI(NAPI_TAG, "Waiting for OnSurfaceCreated callback from system...");
}

/**
 * 模块初始化函数
 */
static napi_value Init(napi_env env, napi_value exports) {
    OHOS_LOGI(NAPI_TAG, "========== NAPI Module Init ==========");
    
    // 尝试从 exports 获取 XComponent（当使用 libraryname 时）
    TryGetXComponentFromExports(env, exports);
    
    // 注册导出函数
    napi_property_descriptor desc[] = {
        {"initEmulator", nullptr, InitEmulator, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"shutdownEmulator", nullptr, ShutdownEmulator, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"loadGame", nullptr, LoadGame, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"initSystemProperties", nullptr, InitSystemProperties, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setSafeInsets", nullptr, SetSafeInsets, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setAudioConfig", nullptr, SetAudioConfig, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"sendTouchEvent", nullptr, SendTouchEvent, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"sendKeyEvent", nullptr, NapiPPSSPP::SendKeyEvent, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"sendAxisEvent", nullptr, NapiPPSSPP::SendAxisEvent, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"sendMultiAxisEvent", nullptr, NapiPPSSPP::SendMultiAxisEvent, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"vibrate", nullptr, NapiPPSSPP::Vibrate, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setVibrationCallback", nullptr, NapiPPSSPP::SetVibrationCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"checkVibrationConfig", nullptr, NapiPPSSPP::CheckVibrationConfig, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setOpenUrlCallback", nullptr, NapiPPSSPP::SetOpenUrlCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setBrowseImageCallback", nullptr, NapiPPSSPP::SetBrowseImageCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onImageSelected", nullptr, NapiPPSSPP::OnImageSelected, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setBrowseFileCallback", nullptr, NapiPPSSPP::SetBrowseFileCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onFileSelected", nullptr, NapiPPSSPP::OnFileSelected, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setBrowseFolderCallback", nullptr, NapiPPSSPP::SetBrowseFolderCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onFolderSelected", nullptr, NapiPPSSPP::OnFolderSelected, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setInputTextCallback", nullptr, NapiPPSSPP::SetInputTextCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onInputTextCompleted", nullptr, NapiPPSSPP::OnInputTextCompleted, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setToastCallback", nullptr, NapiPPSSPP::SetToastCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setClipboardCallback", nullptr, NapiPPSSPP::SetClipboardCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setKeepScreenOnCallback", nullptr, NapiPPSSPP::SetKeepScreenOnCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setShareTextCallback", nullptr, NapiPPSSPP::SetShareTextCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setShowFileInFolderCallback", nullptr, NapiPPSSPP::SetShowFileInFolderCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setRestartAppCallback", nullptr, NapiPPSSPP::SetRestartAppCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setExitAppCallback", nullptr, NapiPPSSPP::SetExitAppCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setRecreateActivityCallback", nullptr, NapiPPSSPP::SetRecreateActivityCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setImmersiveModeCallback", nullptr, NapiPPSSPP::SetImmersiveModeCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setScreenRotationCallback", nullptr, NapiPPSSPP::SetScreenRotationCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setShowKeyboardCallback", nullptr, NapiPPSSPP::SetShowKeyboardCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    
    napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_TAG, "Failed to define NAPI properties");
        return nullptr;
    }
    
    OHOS_LOGI(NAPI_TAG, "NAPI module initialized");
    return exports;
}

/**
 * 模块描述符
 */
static napi_module ppssppModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "ppsspp_ohos",
    .nm_priv = nullptr,
    .reserved = {0},
};

/**
 * 模块注册
 */
extern "C" __attribute__((constructor)) void RegisterPPSSPPModule() {
    OHOS_LOGI(NAPI_TAG, "Registering PPSSPP module...");
    napi_module_register(&ppssppModule);
}
