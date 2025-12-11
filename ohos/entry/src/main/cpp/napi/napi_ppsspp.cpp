/**
 * PPSSPP NAPI 接口实现
 */

#include "napi_ppsspp.h"
#include "../ohos_app.h"
#include "../ohos_input.h"
#include "../ohos_hilog.h"  // 使用自定义的 hilog 包装器，避免 LogLevel 冲突
#include "../ohos_vibration.h"
#include "Core/Config.h"
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

} // namespace NapiPPSSPP
