/**
 * PPSSPP NAPI 接口实现
 */

#include "napi_ppsspp.h"
#include "../ohos_app.h"
#include "../ohos_input.h"
#include "../ohos_hilog.h"  // 使用自定义的 hilog 包装器，避免 LogLevel 冲突
#include "../ohos_vibration.h"
#include "Core/Config.h"
#include "Common/System/Request.h"  // for g_requestManager
#include <string>

#define NAPI_PPSSPP_TAG "PPSSPP_NAPI_PPSSPP"

namespace NapiPPSSPP {

// 辅助函数：从 NAPI 获取字符串参数
static std::string GetStringArg(napi_env env, napi_callback_info info, size_t index) {
    size_t argc = 10;
    napi_value args[10];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (index >= argc) {
        return "";
    }
    
    size_t str_size = 0;
    napi_get_value_string_utf8(env, args[index], nullptr, 0, &str_size);
    
    std::string result(str_size, '\0');
    napi_get_value_string_utf8(env, args[index], &result[0], str_size + 1, &str_size);
    
    return result;
}

// 辅助函数：从 NAPI 获取整数参数
static int32_t GetInt32Arg(napi_env env, napi_callback_info info, size_t index) {
    size_t argc = 10;
    napi_value args[10];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (index >= argc) {
        return 0;
    }
    
    int32_t value = 0;
    napi_get_value_int32(env, args[index], &value);
    return value;
}

// 辅助函数：从 NAPI 获取布尔参数
static bool GetBoolArg(napi_env env, napi_callback_info info, size_t index) {
    size_t argc = 10;
    napi_value args[10];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (index >= argc) {
        return false;
    }
    
    bool value = false;
    napi_get_value_bool(env, args[index], &value);
    return value;
}

napi_value InitEmulator(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "InitEmulator called");
    
    // 初始化各个子系统
    bool success = OhosApp::Initialize();
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

napi_value ShutdownEmulator(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShutdownEmulator called");
    
    OhosApp::Shutdown();
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value LoadGame(napi_env env, napi_callback_info info) {
    std::string gamePath = GetStringArg(env, info, 0);
    OHOS_LOGI(NAPI_PPSSPP_TAG, "LoadGame called: %{public}s", gamePath.c_str());
    
    bool success = OhosApp::LoadGame(gamePath);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

napi_value RunFrame(napi_env env, napi_callback_info info) {
    OhosApp::RunFrame();
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value PauseEmulator(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "PauseEmulator called");
    
    OhosApp::Pause();
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value ResumeEmulator(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ResumeEmulator called");
    
    OhosApp::Resume();
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value SendTouchEvent(napi_env env, napi_callback_info info) {
    int32_t x = GetInt32Arg(env, info, 0);
    int32_t y = GetInt32Arg(env, info, 1);
    int32_t action = GetInt32Arg(env, info, 2);
    
    OHOS_LOGD(NAPI_PPSSPP_TAG, "SendTouchEvent: x=%{public}d, y=%{public}d, action=%{public}d", x, y, action);
    
    OhosInput::HandleTouchEvent(static_cast<float>(x), static_cast<float>(y), 
                                static_cast<OhosInput::TouchAction>(action), 0);
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value SendKeyEvent(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    int32_t keyCode = 0;
    bool isDown = false;
    bool isRepeat = false;
    int32_t deviceId = 0;
    
    if (argc >= 1) napi_get_value_int32(env, args[0], &keyCode);
    if (argc >= 2) napi_get_value_bool(env, args[1], &isDown);
    if (argc >= 3) napi_get_value_bool(env, args[2], &isRepeat);
    if (argc >= 4) napi_get_value_int32(env, args[3], &deviceId);
    
    OHOS_LOGD(NAPI_PPSSPP_TAG, "SendKeyEvent: keyCode=%{public}d, isDown=%{public}d, isRepeat=%{public}d, deviceId=%{public}d", 
              keyCode, isDown, isRepeat, deviceId);
    
    bool consumed = OhosInput::HandleKeyEvent(keyCode, isDown, isRepeat, deviceId);
    
    napi_value result;
    napi_get_boolean(env, consumed, &result);
    return result;
}

napi_value SendAxisEvent(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        OHOS_LOGW(NAPI_PPSSPP_TAG, "SendAxisEvent: requires 3 arguments (deviceId, axisId, value)");
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    }
    
    int32_t deviceId = 0;
    int32_t axisId = 0;
    double value = 0.0;
    
    napi_get_value_int32(env, args[0], &deviceId);
    napi_get_value_int32(env, args[1], &axisId);
    napi_get_value_double(env, args[2], &value);
    
    OHOS_LOGD(NAPI_PPSSPP_TAG, "SendAxisEvent: deviceId=%{public}d, axisId=%{public}d, value=%{public}.3f", 
              deviceId, axisId, value);
    
    OhosInput::HandleAxisEvent(deviceId, axisId, static_cast<float>(value));
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value SendMultiAxisEvent(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        OHOS_LOGW(NAPI_PPSSPP_TAG, "SendMultiAxisEvent: requires 3 arguments (deviceId, axisIds[], values[])");
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    }
    
    int32_t deviceId = 0;
    napi_get_value_int32(env, args[0], &deviceId);
    
    // 获取数组长度
    uint32_t axisCount = 0;
    napi_get_array_length(env, args[1], &axisCount);
    
    if (axisCount == 0) {
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    }
    
    // 读取轴 ID 和值
    int* axisIds = new int[axisCount];
    float* values = new float[axisCount];
    
    for (uint32_t i = 0; i < axisCount; i++) {
        napi_value axisIdVal, valueVal;
        napi_get_element(env, args[1], i, &axisIdVal);
        napi_get_element(env, args[2], i, &valueVal);
        
        int32_t axisId = 0;
        double value = 0.0;
        napi_get_value_int32(env, axisIdVal, &axisId);
        napi_get_value_double(env, valueVal, &value);
        
        axisIds[i] = axisId;
        values[i] = static_cast<float>(value);
    }
    
    OhosInput::HandleMultiAxisEvent(deviceId, axisIds, values, axisCount);
    
    delete[] axisIds;
    delete[] values;
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value GetConfig(napi_env env, napi_callback_info info) {
    std::string key = GetStringArg(env, info, 0);
    OHOS_LOGD(NAPI_PPSSPP_TAG, "GetConfig: key=%{public}s", key.c_str());
    
    // TODO: 实现配置获取
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value SetConfig(napi_env env, napi_callback_info info) {
    std::string key = GetStringArg(env, info, 0);
    std::string value = GetStringArg(env, info, 1);
    
    OHOS_LOGD(NAPI_PPSSPP_TAG, "SetConfig: key=%{public}s, value=%{public}s", key.c_str(), value.c_str());
    
    // TODO: 实现配置设置
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value Vibrate(napi_env env, napi_callback_info info) {
    int32_t duration = GetInt32Arg(env, info, 0);
    
    // 调用震动管理器，它会通过回调调用 ArkTS 层
    bool success = OhosVibration::Vibrate(duration);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

// 全局变量保存 ArkTS 回调函数
static napi_env g_vibrationEnv = nullptr;
static napi_ref g_vibrationCallbackRef = nullptr;

napi_value SetVibrationCallback(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 检查参数是否为函数
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 清理之前的回调引用
    if (g_vibrationCallbackRef != nullptr) {
        napi_delete_reference(g_vibrationEnv, g_vibrationCallbackRef);
        g_vibrationCallbackRef = nullptr;
    }
    
    // 保存新的回调函数引用
    g_vibrationEnv = env;
    napi_create_reference(env, args[0], 1, &g_vibrationCallbackRef);
    
    // 设置 C++ 层的回调函数
    OhosVibration::SetVibrationCallback([](int duration) -> bool {
        if (g_vibrationEnv == nullptr || g_vibrationCallbackRef == nullptr) {
            return false;
        }
        
        // 获取回调函数
        napi_value callback;
        napi_status status = napi_get_reference_value(g_vibrationEnv, g_vibrationCallbackRef, &callback);
        if (status != napi_ok) {
            return false;
        }
        
        // 创建参数
        napi_value args[1];
        status = napi_create_int32(g_vibrationEnv, duration, &args[0]);
        if (status != napi_ok) {
            return false;
        }
        
        // 调用回调函数
        napi_value result;
        status = napi_call_function(g_vibrationEnv, nullptr, callback, 1, args, &result);
        if (status != napi_ok) {
            return false;
        }
        
        // 获取返回值
        bool success = false;
        status = napi_get_value_bool(g_vibrationEnv, result, &success);
        if (status != napi_ok) {
            return false;
        }
        
        return success;
    });
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

napi_value CheckVibrationConfig(napi_env env, napi_callback_info info) {
    // 返回当前的震动配置状态（不强制修改）
    napi_value result;
    napi_get_boolean(env, g_Config.bHapticFeedback, &result);
    return result;
}

// ============================================================================
// 打开 URL 功能 - 使用线程安全函数
// ============================================================================

// 线程安全函数，用于从任意线程调用 ArkTS 回调
static napi_threadsafe_function g_openUrlTsFunc = nullptr;

// 线程安全函数的调用回调 - 在主线程执行
static void OpenUrlCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OpenUrlCallJs: invalid env or callback");
        if (data) {
            delete static_cast<std::string*>(data);
        }
        return;
    }
    
    std::string* urlPtr = static_cast<std::string*>(data);
    if (urlPtr == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OpenUrlCallJs: url is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "OpenUrlCallJs: opening URL on main thread: %{public}s", urlPtr->c_str());
    
    // 创建 URL 字符串参数
    napi_value urlArg;
    napi_status status = napi_create_string_utf8(env, urlPtr->c_str(), urlPtr->length(), &urlArg);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OpenUrlCallJs: failed to create URL string");
        delete urlPtr;
        return;
    }
    
    // 调用回调函数
    napi_value result;
    napi_value args[1] = { urlArg };
    status = napi_call_function(env, nullptr, js_callback, 1, args, &result);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OpenUrlCallJs: failed to call callback function, status=%{public}d", status);
    } else {
        OHOS_LOGI(NAPI_PPSSPP_TAG, "OpenUrlCallJs: callback called successfully");
    }
    
    // 清理
    delete urlPtr;
}

napi_value SetOpenUrlCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetOpenUrlCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetOpenUrlCallback: missing callback argument");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 检查参数是否为函数
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetOpenUrlCallback: argument is not a function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 清理之前的线程安全函数
    if (g_openUrlTsFunc != nullptr) {
        napi_release_threadsafe_function(g_openUrlTsFunc, napi_tsfn_release);
        g_openUrlTsFunc = nullptr;
    }
    
    // 创建资源名称
    napi_value resourceName;
    napi_create_string_utf8(env, "OpenUrlCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    // 创建线程安全函数
    napi_status status = napi_create_threadsafe_function(
        env,
        args[0],           // JS 回调函数
        nullptr,           // async_resource
        resourceName,      // async_resource_name
        0,                 // max_queue_size (0 = 无限制)
        1,                 // initial_thread_count
        nullptr,           // thread_finalize_data
        nullptr,           // thread_finalize_cb
        nullptr,           // context
        OpenUrlCallJs,     // call_js_cb
        &g_openUrlTsFunc   // result
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetOpenUrlCallback: failed to create threadsafe function, status=%{public}d", status);
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "OpenUrl threadsafe callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool OpenUrl(const std::string& url) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "OpenUrl called: %{public}s", url.c_str());
    
    if (g_openUrlTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OpenUrl: threadsafe callback not registered");
        return false;
    }
    
    // 复制 URL 字符串，因为需要传递给主线程
    std::string* urlCopy = new std::string(url);
    
    // 调用线程安全函数，将请求发送到主线程
    napi_status status = napi_call_threadsafe_function(g_openUrlTsFunc, urlCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OpenUrl: failed to call threadsafe function, status=%{public}d", status);
        delete urlCopy;
        return false;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "OpenUrl: request sent to main thread");
    return true;
}

// ============================================================================
// 浏览图片功能 - 使用线程安全函数
// ============================================================================

// 线程安全函数，用于从任意线程调用 ArkTS 图片选择器
static napi_threadsafe_function g_browseImageTsFunc = nullptr;

// 线程安全函数的调用回调 - 在主线程执行
static void BrowseImageCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseImageCallJs: invalid env or callback");
        return;
    }
    
    int* requestIdPtr = static_cast<int*>(data);
    if (requestIdPtr == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseImageCallJs: requestId is null");
        return;
    }
    
    int requestId = *requestIdPtr;
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseImageCallJs: opening image picker on main thread, requestId=%{public}d", requestId);
    
    // 创建 requestId 参数
    napi_value requestIdArg;
    napi_status status = napi_create_int32(env, requestId, &requestIdArg);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseImageCallJs: failed to create requestId");
        delete requestIdPtr;
        return;
    }
    
    // 调用回调函数
    napi_value result;
    napi_value args[1] = { requestIdArg };
    status = napi_call_function(env, nullptr, js_callback, 1, args, &result);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseImageCallJs: failed to call callback function, status=%{public}d", status);
    } else {
        OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseImageCallJs: callback called successfully");
    }
    
    // 清理
    delete requestIdPtr;
}

napi_value SetBrowseImageCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetBrowseImageCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetBrowseImageCallback: missing callback argument");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 检查参数是否为函数
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetBrowseImageCallback: argument is not a function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 清理之前的线程安全函数
    if (g_browseImageTsFunc != nullptr) {
        napi_release_threadsafe_function(g_browseImageTsFunc, napi_tsfn_release);
        g_browseImageTsFunc = nullptr;
    }
    
    // 创建资源名称
    napi_value resourceName;
    napi_create_string_utf8(env, "BrowseImageCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    // 创建线程安全函数
    napi_status status = napi_create_threadsafe_function(
        env,
        args[0],              // JS 回调函数
        nullptr,              // async_resource
        resourceName,         // async_resource_name
        0,                    // max_queue_size (0 = 无限制)
        1,                    // initial_thread_count
        nullptr,              // thread_finalize_data
        nullptr,              // thread_finalize_cb
        nullptr,              // context
        BrowseImageCallJs,    // call_js_cb
        &g_browseImageTsFunc  // result
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetBrowseImageCallback: failed to create threadsafe function, status=%{public}d", status);
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseImage threadsafe callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool BrowseForImage(int requestId) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseForImage called: requestId=%{public}d", requestId);
    
    if (g_browseImageTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseForImage: threadsafe callback not registered");
        return false;
    }
    
    // 复制 requestId，因为需要传递给主线程
    int* requestIdCopy = new int(requestId);
    
    // 调用线程安全函数，将请求发送到主线程
    napi_status status = napi_call_threadsafe_function(g_browseImageTsFunc, requestIdCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseForImage: failed to call threadsafe function, status=%{public}d", status);
        delete requestIdCopy;
        return false;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseForImage: request sent to main thread");
    return true;
}

napi_value OnImageSelected(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OnImageSelected: requires 3 arguments (requestId, success, path)");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    // 获取 requestId
    int32_t requestId = 0;
    napi_get_value_int32(env, args[0], &requestId);
    
    // 获取 success
    bool success = false;
    napi_get_value_bool(env, args[1], &success);
    
    // 获取 path
    std::string path;
    size_t strSize = 0;
    napi_get_value_string_utf8(env, args[2], nullptr, 0, &strSize);
    if (strSize > 0) {
        path.resize(strSize);
        napi_get_value_string_utf8(env, args[2], &path[0], strSize + 1, &strSize);
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "OnImageSelected: requestId=%{public}d, success=%{public}d, path=%{public}s",
              requestId, success, path.c_str());
    
    // 通知 RequestManager
    if (success && !path.empty()) {
        g_requestManager.PostSystemSuccess(requestId, path.c_str());
    } else {
        g_requestManager.PostSystemFailure(requestId);
    }
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

// ============================================================================
// 浏览文件功能 - 使用线程安全函数
// ============================================================================

// 文件浏览请求数据
struct BrowseFileRequest {
    int requestId;
    int fileType;  // BrowseFileType
};

// 线程安全函数，用于从任意线程调用 ArkTS 文件选择器
static napi_threadsafe_function g_browseFileTsFunc = nullptr;

// 线程安全函数的调用回调 - 在主线程执行
static void BrowseFileCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseFileCallJs: invalid env or callback");
        if (data) delete static_cast<BrowseFileRequest*>(data);
        return;
    }
    
    BrowseFileRequest* request = static_cast<BrowseFileRequest*>(data);
    if (request == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseFileCallJs: request is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseFileCallJs: opening file picker, requestId=%{public}d, fileType=%{public}d", 
              request->requestId, request->fileType);
    
    // 创建参数
    napi_value requestIdArg, fileTypeArg;
    napi_create_int32(env, request->requestId, &requestIdArg);
    napi_create_int32(env, request->fileType, &fileTypeArg);
    
    // 调用回调函数
    napi_value result;
    napi_value args[2] = { requestIdArg, fileTypeArg };
    napi_status status = napi_call_function(env, nullptr, js_callback, 2, args, &result);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseFileCallJs: failed to call callback, status=%{public}d", status);
    }
    
    delete request;
}

napi_value SetBrowseFileCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetBrowseFileCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_browseFileTsFunc != nullptr) {
        napi_release_threadsafe_function(g_browseFileTsFunc, napi_tsfn_release);
        g_browseFileTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "BrowseFileCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        BrowseFileCallJs, &g_browseFileTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetBrowseFileCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseFile callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool BrowseForFile(int requestId, int fileType) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseForFile called: requestId=%{public}d, fileType=%{public}d", requestId, fileType);
    
    if (g_browseFileTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseForFile: callback not registered");
        return false;
    }
    
    BrowseFileRequest* request = new BrowseFileRequest{requestId, fileType};
    
    napi_status status = napi_call_threadsafe_function(g_browseFileTsFunc, request, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseForFile: failed to call threadsafe function");
        delete request;
        return false;
    }
    
    return true;
}

napi_value OnFileSelected(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OnFileSelected: requires 3 arguments");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    int32_t requestId = 0;
    bool success = false;
    std::string path;
    
    napi_get_value_int32(env, args[0], &requestId);
    napi_get_value_bool(env, args[1], &success);
    
    size_t strSize = 0;
    napi_get_value_string_utf8(env, args[2], nullptr, 0, &strSize);
    if (strSize > 0) {
        path.resize(strSize);
        napi_get_value_string_utf8(env, args[2], &path[0], strSize + 1, &strSize);
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "OnFileSelected: requestId=%{public}d, success=%{public}d, path=%{public}s",
              requestId, success, path.c_str());
    
    if (success && !path.empty()) {
        g_requestManager.PostSystemSuccess(requestId, path.c_str());
    } else {
        g_requestManager.PostSystemFailure(requestId);
    }
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

// ============================================================================
// 浏览文件夹功能 - 使用线程安全函数
// ============================================================================

// 线程安全函数，用于从任意线程调用 ArkTS 文件夹选择器
static napi_threadsafe_function g_browseFolderTsFunc = nullptr;

// 线程安全函数的调用回调 - 在主线程执行
static void BrowseFolderCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseFolderCallJs: invalid env or callback");
        if (data) delete static_cast<int*>(data);
        return;
    }
    
    int* requestIdPtr = static_cast<int*>(data);
    if (requestIdPtr == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseFolderCallJs: requestId is null");
        return;
    }
    
    int requestId = *requestIdPtr;
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseFolderCallJs: opening folder picker, requestId=%{public}d", requestId);
    
    napi_value requestIdArg;
    napi_create_int32(env, requestId, &requestIdArg);
    
    napi_value result;
    napi_value args[1] = { requestIdArg };
    napi_status status = napi_call_function(env, nullptr, js_callback, 1, args, &result);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseFolderCallJs: failed to call callback, status=%{public}d", status);
    }
    
    delete requestIdPtr;
}

napi_value SetBrowseFolderCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetBrowseFolderCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_browseFolderTsFunc != nullptr) {
        napi_release_threadsafe_function(g_browseFolderTsFunc, napi_tsfn_release);
        g_browseFolderTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "BrowseFolderCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        BrowseFolderCallJs, &g_browseFolderTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetBrowseFolderCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseFolder callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool BrowseForFolder(int requestId) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "BrowseForFolder called: requestId=%{public}d", requestId);
    
    if (g_browseFolderTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseForFolder: callback not registered");
        return false;
    }
    
    int* requestIdCopy = new int(requestId);
    
    napi_status status = napi_call_threadsafe_function(g_browseFolderTsFunc, requestIdCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "BrowseForFolder: failed to call threadsafe function");
        delete requestIdCopy;
        return false;
    }
    
    return true;
}

napi_value OnFolderSelected(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OnFolderSelected: requires 3 arguments");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    int32_t requestId = 0;
    bool success = false;
    std::string path;
    
    napi_get_value_int32(env, args[0], &requestId);
    napi_get_value_bool(env, args[1], &success);
    
    size_t strSize = 0;
    napi_get_value_string_utf8(env, args[2], nullptr, 0, &strSize);
    if (strSize > 0) {
        path.resize(strSize);
        napi_get_value_string_utf8(env, args[2], &path[0], strSize + 1, &strSize);
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "OnFolderSelected: requestId=%{public}d, success=%{public}d, path=%{public}s",
              requestId, success, path.c_str());
    
    if (success && !path.empty()) {
        g_requestManager.PostSystemSuccess(requestId, path.c_str());
    } else {
        g_requestManager.PostSystemFailure(requestId);
    }
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

// ============================================================================
// 文本输入对话框功能 - 使用线程安全函数
// ============================================================================

// 文本输入请求数据
struct InputTextRequest {
    int requestId;
    std::string title;
    std::string defaultText;
};

// 线程安全函数，用于从任意线程调用 ArkTS 文本输入对话框
static napi_threadsafe_function g_inputTextTsFunc = nullptr;

// 线程安全函数的调用回调 - 在主线程执行
static void InputTextCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "InputTextCallJs: invalid env or callback");
        if (data) delete static_cast<InputTextRequest*>(data);
        return;
    }
    
    InputTextRequest* request = static_cast<InputTextRequest*>(data);
    if (request == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "InputTextCallJs: request is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "InputTextCallJs: showing input dialog, requestId=%{public}d, title=%{public}s", 
              request->requestId, request->title.c_str());
    
    // 创建参数
    napi_value requestIdArg, titleArg, defaultTextArg;
    napi_create_int32(env, request->requestId, &requestIdArg);
    napi_create_string_utf8(env, request->title.c_str(), request->title.length(), &titleArg);
    napi_create_string_utf8(env, request->defaultText.c_str(), request->defaultText.length(), &defaultTextArg);
    
    // 调用回调函数
    napi_value result;
    napi_value args[3] = { requestIdArg, titleArg, defaultTextArg };
    napi_status status = napi_call_function(env, nullptr, js_callback, 3, args, &result);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "InputTextCallJs: failed to call callback, status=%{public}d", status);
    }
    
    delete request;
}

napi_value SetInputTextCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetInputTextCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_inputTextTsFunc != nullptr) {
        napi_release_threadsafe_function(g_inputTextTsFunc, napi_tsfn_release);
        g_inputTextTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "InputTextCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        InputTextCallJs, &g_inputTextTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetInputTextCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "InputText callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool ShowInputTextDialog(int requestId, const std::string& title, const std::string& defaultText) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShowInputTextDialog called: requestId=%{public}d, title=%{public}s", requestId, title.c_str());
    
    if (g_inputTextTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowInputTextDialog: callback not registered");
        return false;
    }
    
    InputTextRequest* request = new InputTextRequest{requestId, title, defaultText};
    
    napi_status status = napi_call_threadsafe_function(g_inputTextTsFunc, request, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowInputTextDialog: failed to call threadsafe function");
        delete request;
        return false;
    }
    
    return true;
}

napi_value OnInputTextCompleted(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "OnInputTextCompleted: requires 3 arguments");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    int32_t requestId = 0;
    bool success = false;
    std::string text;
    
    napi_get_value_int32(env, args[0], &requestId);
    napi_get_value_bool(env, args[1], &success);
    
    size_t strSize = 0;
    napi_get_value_string_utf8(env, args[2], nullptr, 0, &strSize);
    if (strSize > 0) {
        text.resize(strSize);
        napi_get_value_string_utf8(env, args[2], &text[0], strSize + 1, &strSize);
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "OnInputTextCompleted: requestId=%{public}d, success=%{public}d, text=%{public}s",
              requestId, success, text.c_str());
    
    if (success) {
        g_requestManager.PostSystemSuccess(requestId, text.c_str());
    } else {
        g_requestManager.PostSystemFailure(requestId);
    }
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

// ============================================================================
// Toast 功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_toastTsFunc = nullptr;

static void ToastCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ToastCallJs: invalid env or callback");
        if (data) delete static_cast<std::string*>(data);
        return;
    }
    
    std::string* message = static_cast<std::string*>(data);
    if (message == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ToastCallJs: message is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ToastCallJs: showing toast: %{public}s", message->c_str());
    
    napi_value messageArg;
    napi_create_string_utf8(env, message->c_str(), message->length(), &messageArg);
    
    napi_value result;
    napi_value args[1] = { messageArg };
    napi_call_function(env, nullptr, js_callback, 1, args, &result);
    
    delete message;
}

napi_value SetToastCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetToastCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_toastTsFunc != nullptr) {
        napi_release_threadsafe_function(g_toastTsFunc, napi_tsfn_release);
        g_toastTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "ToastCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        ToastCallJs, &g_toastTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetToastCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "Toast callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool ShowToast(const std::string& message) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShowToast called: %{public}s", message.c_str());
    
    if (g_toastTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowToast: callback not registered");
        return false;
    }
    
    std::string* messageCopy = new std::string(message);
    
    napi_status status = napi_call_threadsafe_function(g_toastTsFunc, messageCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowToast: failed to call threadsafe function");
        delete messageCopy;
        return false;
    }
    
    return true;
}

// ============================================================================
// 剪贴板功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_clipboardTsFunc = nullptr;

static void ClipboardCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ClipboardCallJs: invalid env or callback");
        if (data) delete static_cast<std::string*>(data);
        return;
    }
    
    std::string* text = static_cast<std::string*>(data);
    if (text == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ClipboardCallJs: text is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ClipboardCallJs: copying to clipboard");
    
    napi_value textArg;
    napi_create_string_utf8(env, text->c_str(), text->length(), &textArg);
    
    napi_value result;
    napi_value args[1] = { textArg };
    napi_call_function(env, nullptr, js_callback, 1, args, &result);
    
    delete text;
}

napi_value SetClipboardCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetClipboardCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_clipboardTsFunc != nullptr) {
        napi_release_threadsafe_function(g_clipboardTsFunc, napi_tsfn_release);
        g_clipboardTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "ClipboardCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        ClipboardCallJs, &g_clipboardTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetClipboardCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "Clipboard callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool CopyToClipboard(const std::string& text) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "CopyToClipboard called");
    
    if (g_clipboardTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "CopyToClipboard: callback not registered");
        return false;
    }
    
    std::string* textCopy = new std::string(text);
    
    napi_status status = napi_call_threadsafe_function(g_clipboardTsFunc, textCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "CopyToClipboard: failed to call threadsafe function");
        delete textCopy;
        return false;
    }
    
    return true;
}

// ============================================================================
// 屏幕常亮功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_keepScreenOnTsFunc = nullptr;

static void KeepScreenOnCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "KeepScreenOnCallJs: invalid env or callback");
        if (data) delete static_cast<bool*>(data);
        return;
    }
    
    bool* keepOn = static_cast<bool*>(data);
    if (keepOn == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "KeepScreenOnCallJs: keepOn is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "KeepScreenOnCallJs: setting keep screen on: %{public}d", *keepOn);
    
    napi_value keepOnArg;
    napi_get_boolean(env, *keepOn, &keepOnArg);
    
    napi_value result;
    napi_value args[1] = { keepOnArg };
    napi_call_function(env, nullptr, js_callback, 1, args, &result);
    
    delete keepOn;
}

napi_value SetKeepScreenOnCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetKeepScreenOnCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_keepScreenOnTsFunc != nullptr) {
        napi_release_threadsafe_function(g_keepScreenOnTsFunc, napi_tsfn_release);
        g_keepScreenOnTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "KeepScreenOnCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        KeepScreenOnCallJs, &g_keepScreenOnTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetKeepScreenOnCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "KeepScreenOn callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool SetKeepScreenOn(bool keepOn) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetKeepScreenOn called: %{public}d", keepOn);
    
    if (g_keepScreenOnTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetKeepScreenOn: callback not registered");
        return false;
    }
    
    bool* keepOnCopy = new bool(keepOn);
    
    napi_status status = napi_call_threadsafe_function(g_keepScreenOnTsFunc, keepOnCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetKeepScreenOn: failed to call threadsafe function");
        delete keepOnCopy;
        return false;
    }
    
    return true;
}

// ============================================================================
// 分享文本功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_shareTextTsFunc = nullptr;

static void ShareTextCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShareTextCallJs: invalid env or callback");
        if (data) delete static_cast<std::string*>(data);
        return;
    }
    
    std::string* text = static_cast<std::string*>(data);
    if (text == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShareTextCallJs: text is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShareTextCallJs: sharing text");
    
    napi_value textArg;
    napi_create_string_utf8(env, text->c_str(), text->length(), &textArg);
    
    napi_value result;
    napi_value args[1] = { textArg };
    napi_call_function(env, nullptr, js_callback, 1, args, &result);
    
    delete text;
}

napi_value SetShareTextCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetShareTextCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_shareTextTsFunc != nullptr) {
        napi_release_threadsafe_function(g_shareTextTsFunc, napi_tsfn_release);
        g_shareTextTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "ShareTextCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        ShareTextCallJs, &g_shareTextTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetShareTextCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShareText callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool ShareText(const std::string& text) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShareText called");
    
    if (g_shareTextTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShareText: callback not registered");
        return false;
    }
    
    std::string* textCopy = new std::string(text);
    
    napi_status status = napi_call_threadsafe_function(g_shareTextTsFunc, textCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShareText: failed to call threadsafe function");
        delete textCopy;
        return false;
    }
    
    return true;
}

// ============================================================================
// 显示文件位置功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_showFileInFolderTsFunc = nullptr;

static void ShowFileInFolderCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowFileInFolderCallJs: invalid env or callback");
        if (data) delete static_cast<std::string*>(data);
        return;
    }
    
    std::string* path = static_cast<std::string*>(data);
    if (path == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowFileInFolderCallJs: path is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShowFileInFolderCallJs: showing file: %{public}s", path->c_str());
    
    napi_value pathArg;
    napi_create_string_utf8(env, path->c_str(), path->length(), &pathArg);
    
    napi_value result;
    napi_value args[1] = { pathArg };
    napi_call_function(env, nullptr, js_callback, 1, args, &result);
    
    delete path;
}

napi_value SetShowFileInFolderCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetShowFileInFolderCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_showFileInFolderTsFunc != nullptr) {
        napi_release_threadsafe_function(g_showFileInFolderTsFunc, napi_tsfn_release);
        g_showFileInFolderTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "ShowFileInFolderCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        ShowFileInFolderCallJs, &g_showFileInFolderTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetShowFileInFolderCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShowFileInFolder callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool ShowFileInFolder(const std::string& path) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShowFileInFolder called: %{public}s", path.c_str());
    
    if (g_showFileInFolderTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowFileInFolder: callback not registered");
        return false;
    }
    
    std::string* pathCopy = new std::string(path);
    
    napi_status status = napi_call_threadsafe_function(g_showFileInFolderTsFunc, pathCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowFileInFolder: failed to call threadsafe function");
        delete pathCopy;
        return false;
    }
    
    return true;
}

// ============================================================================
// 重启应用功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_restartAppTsFunc = nullptr;

static void RestartAppCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "RestartAppCallJs: invalid env or callback");
        if (data) delete static_cast<std::string*>(data);
        return;
    }
    
    std::string* params = static_cast<std::string*>(data);
    if (params == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "RestartAppCallJs: params is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "RestartAppCallJs: restarting app with params: %{public}s", params->c_str());
    
    napi_value paramsArg;
    napi_create_string_utf8(env, params->c_str(), params->length(), &paramsArg);
    
    napi_value result;
    napi_value args[1] = { paramsArg };
    napi_call_function(env, nullptr, js_callback, 1, args, &result);
    
    delete params;
}

napi_value SetRestartAppCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetRestartAppCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_restartAppTsFunc != nullptr) {
        napi_release_threadsafe_function(g_restartAppTsFunc, napi_tsfn_release);
        g_restartAppTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "RestartAppCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        RestartAppCallJs, &g_restartAppTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetRestartAppCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "RestartApp callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool RestartApp(const std::string& params) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "RestartApp called with params: %{public}s", params.c_str());
    
    if (g_restartAppTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "RestartApp: callback not registered");
        return false;
    }
    
    std::string* paramsCopy = new std::string(params);
    
    napi_status status = napi_call_threadsafe_function(g_restartAppTsFunc, paramsCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "RestartApp: failed to call threadsafe function");
        delete paramsCopy;
        return false;
    }
    
    return true;
}

// ============================================================================
// 退出应用功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_exitAppTsFunc = nullptr;

static void ExitAppCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ExitAppCallJs: invalid env or callback");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ExitAppCallJs: exiting app");
    
    napi_value result;
    napi_call_function(env, nullptr, js_callback, 0, nullptr, &result);
}

napi_value SetExitAppCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetExitAppCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_exitAppTsFunc != nullptr) {
        napi_release_threadsafe_function(g_exitAppTsFunc, napi_tsfn_release);
        g_exitAppTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "ExitAppCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        ExitAppCallJs, &g_exitAppTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetExitAppCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ExitApp callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool ExitApp() {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ExitApp called");
    
    if (g_exitAppTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ExitApp: callback not registered");
        return false;
    }
    
    napi_status status = napi_call_threadsafe_function(g_exitAppTsFunc, nullptr, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ExitApp: failed to call threadsafe function");
        return false;
    }
    
    return true;
}

// ============================================================================
// 重建 Activity 功能 - 使用线程安全函数
// 用于显示设置变更（如分辨率）
// ============================================================================

static napi_threadsafe_function g_recreateActivityTsFunc = nullptr;

static void RecreateActivityCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "RecreateActivityCallJs: invalid env or callback");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "RecreateActivityCallJs: recreating activity");
    
    napi_value result;
    napi_call_function(env, nullptr, js_callback, 0, nullptr, &result);
}

napi_value SetRecreateActivityCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetRecreateActivityCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_recreateActivityTsFunc != nullptr) {
        napi_release_threadsafe_function(g_recreateActivityTsFunc, napi_tsfn_release);
        g_recreateActivityTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "RecreateActivityCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        RecreateActivityCallJs, &g_recreateActivityTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetRecreateActivityCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "RecreateActivity callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool RecreateActivity() {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "RecreateActivity called");
    
    if (g_recreateActivityTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "RecreateActivity: callback not registered");
        return false;
    }
    
    napi_status status = napi_call_threadsafe_function(g_recreateActivityTsFunc, nullptr, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "RecreateActivity: failed to call threadsafe function");
        return false;
    }
    
    return true;
}

// ============================================================================
// 沉浸模式功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_immersiveModeTsFunc = nullptr;

static void ImmersiveModeCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ImmersiveModeCallJs: invalid env or callback");
        if (data) delete static_cast<bool*>(data);
        return;
    }
    
    bool* immersive = static_cast<bool*>(data);
    if (immersive == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ImmersiveModeCallJs: immersive is null");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ImmersiveModeCallJs: setting immersive mode: %{public}d", *immersive);
    
    napi_value immersiveArg;
    napi_get_boolean(env, *immersive, &immersiveArg);
    
    napi_value result;
    napi_value args[1] = { immersiveArg };
    napi_call_function(env, nullptr, js_callback, 1, args, &result);
    
    delete immersive;
}

napi_value SetImmersiveModeCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetImmersiveModeCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_immersiveModeTsFunc != nullptr) {
        napi_release_threadsafe_function(g_immersiveModeTsFunc, napi_tsfn_release);
        g_immersiveModeTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "ImmersiveModeCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        ImmersiveModeCallJs, &g_immersiveModeTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetImmersiveModeCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ImmersiveMode callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool SetImmersiveMode(bool immersive) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetImmersiveMode called: %{public}d", immersive);
    
    if (g_immersiveModeTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetImmersiveMode: callback not registered");
        return false;
    }
    
    bool* immersiveCopy = new bool(immersive);
    
    napi_status status = napi_call_threadsafe_function(g_immersiveModeTsFunc, immersiveCopy, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetImmersiveMode: failed to call threadsafe function");
        delete immersiveCopy;
        return false;
    }
    
    return true;
}

// ============================================================================
// 屏幕旋转功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_screenRotationTsFunc = nullptr;

static void ScreenRotationCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ScreenRotationCallJs: invalid env or callback");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ScreenRotationCallJs: updating screen rotation");
    
    napi_value result;
    napi_call_function(env, nullptr, js_callback, 0, nullptr, &result);
}

napi_value SetScreenRotationCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetScreenRotationCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_screenRotationTsFunc != nullptr) {
        napi_release_threadsafe_function(g_screenRotationTsFunc, napi_tsfn_release);
        g_screenRotationTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "ScreenRotationCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        ScreenRotationCallJs, &g_screenRotationTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetScreenRotationCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ScreenRotation callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool UpdateScreenRotation() {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "UpdateScreenRotation called");
    
    if (g_screenRotationTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "UpdateScreenRotation: callback not registered");
        return false;
    }
    
    napi_status status = napi_call_threadsafe_function(g_screenRotationTsFunc, nullptr, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "UpdateScreenRotation: failed to call threadsafe function");
        return false;
    }
    
    return true;
}

// ============================================================================
// 显示软键盘功能 - 使用线程安全函数
// ============================================================================

static napi_threadsafe_function g_showKeyboardTsFunc = nullptr;

static void ShowKeyboardCallJs(napi_env env, napi_value js_callback, void* context, void* data) {
    if (env == nullptr || js_callback == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowKeyboardCallJs: invalid env or callback");
        return;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShowKeyboardCallJs: showing keyboard");
    
    napi_value result;
    napi_call_function(env, nullptr, js_callback, 0, nullptr, &result);
}

napi_value SetShowKeyboardCallback(napi_env env, napi_callback_info info) {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "SetShowKeyboardCallback called");
    
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    napi_valuetype valueType;
    napi_typeof(env, args[0], &valueType);
    if (valueType != napi_function) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    if (g_showKeyboardTsFunc != nullptr) {
        napi_release_threadsafe_function(g_showKeyboardTsFunc, napi_tsfn_release);
        g_showKeyboardTsFunc = nullptr;
    }
    
    napi_value resourceName;
    napi_create_string_utf8(env, "ShowKeyboardCallback", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_status status = napi_create_threadsafe_function(
        env, args[0], nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr,
        ShowKeyboardCallJs, &g_showKeyboardTsFunc
    );
    
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "SetShowKeyboardCallback: failed to create threadsafe function");
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShowKeyboard callback registered successfully");
    
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

bool ShowKeyboard() {
    OHOS_LOGI(NAPI_PPSSPP_TAG, "ShowKeyboard called");
    
    if (g_showKeyboardTsFunc == nullptr) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowKeyboard: callback not registered");
        return false;
    }
    
    napi_status status = napi_call_threadsafe_function(g_showKeyboardTsFunc, nullptr, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        OHOS_LOGE(NAPI_PPSSPP_TAG, "ShowKeyboard: failed to call threadsafe function");
        return false;
    }
    
    return true;
}

} // namespace NapiPPSSPP
