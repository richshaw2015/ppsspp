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
    int32_t keyCode = GetInt32Arg(env, info, 0);
    bool isDown = GetBoolArg(env, info, 1);
    
    OHOS_LOGD(NAPI_PPSSPP_TAG, "SendKeyEvent: keyCode=%{public}d, isDown=%{public}d", keyCode, isDown);
    
    OhosInput::HandleKeyEvent(keyCode, isDown);
    
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

} // namespace NapiPPSSPP
