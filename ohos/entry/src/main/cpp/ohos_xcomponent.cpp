/**
 * 鸿蒙 XComponent 集成实现
 * 参考 android/jni/app-android.cpp 的渲染循环
 */

#include "ohos_xcomponent.h"
#include "ohos_gl_context.h"
#include "ohos_hilog.h"  // 使用自定义的 hilog 包装器，避免 LogLevel 冲突
#include "ohos_vibration.h"
#include "Common/Log.h"
#include "Common/System/NativeApp.h"
#include "Common/System/Display.h"
#include "Common/System/System.h"
#include "Common/Input/InputState.h"
#include "Common/TimeUtil.h"
#include "Core/System.h"
#include "Core/Config.h"
#include "Core/ConfigValues.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

#define XCOMP_TAG "PPSSPP_XComponent"

namespace OhosXComponent {

static OhosGLContext* g_glContext = nullptr;
static OH_NativeXComponent* g_component = nullptr;
static void* g_nativeWindow = nullptr;
static std::thread g_renderThread;
static std::atomic<bool> g_renderLoopRunning(false);
static std::atomic<bool> g_exitRenderLoop(false);
static std::mutex g_surfaceMutex;
static int g_surfaceWidth = 0;
static int g_surfaceHeight = 0;
static bool g_rendererInited = false;

// 帧率统计
static std::atomic<int> g_frameCount(0);
static std::atomic<double> g_lastFpsTime(0.0);
static std::atomic<double> g_currentFps(0.0);
static std::atomic<double> g_lastFrameTime(0.0);

// XComponent 回调结构体
static void OnSurfaceCreatedCB(OH_NativeXComponent* component, void* window) {
    OnSurfaceCreated(component, window);
}

static void OnSurfaceChangedCB(OH_NativeXComponent* component, void* window) {
    OnSurfaceChanged(component, window);
}

static void OnSurfaceDestroyedCB(OH_NativeXComponent* component, void* window) {
    OnSurfaceDestroyed(component, window);
}

static void DispatchTouchEventCB(OH_NativeXComponent* component, void* window) {
    OnTouchEvent(component, window);
}

bool Initialize(OH_NativeXComponent* component) {
    if (!component) {
        ERROR_LOG(Log::G3D, "XComponent is null");
        return false;
    }
    
    // 防止重复初始化
    if (g_component != nullptr) {
        WARN_LOG(Log::G3D, "XComponent already initialized, skipping");
        return true;
    }
    
    INFO_LOG(Log::G3D, "Initializing XComponent");
    g_component = component;
    
    // 注册回调 - 注意：回调已经在 napi_init.cpp 中注册了，这里不需要重复注册
    // OH_NativeXComponent_Callback callback;
    // callback.OnSurfaceCreated = OnSurfaceCreatedCB;
    // callback.OnSurfaceChanged = OnSurfaceChangedCB;
    // callback.OnSurfaceDestroyed = OnSurfaceDestroyedCB;
    // callback.DispatchTouchEvent = DispatchTouchEventCB;
    // OH_NativeXComponent_RegisterCallback(component, &callback);
    
    // 创建 GL 上下文（只创建一次）
    if (!g_glContext) {
        g_glContext = new OhosGLContext();
    }
    
    INFO_LOG(Log::G3D, "XComponent initialized");
    return true;
}

void Shutdown() {
    INFO_LOG(Log::G3D, "Shutting down XComponent");
    
    StopRenderLoop();
    
    if (g_glContext) {
        g_glContext->Shutdown();
        delete g_glContext;
        g_glContext = nullptr;
    }
    
    g_component = nullptr;
    g_nativeWindow = nullptr;
}

OhosGLContext* GetGLContext() {
    return g_glContext;
}

void OnSurfaceCreated(OH_NativeXComponent* component, void* window) {
    OHOS_LOGI(XCOMP_TAG, "=== OnSurfaceCreated START ===");
    INFO_LOG(Log::G3D, "XComponent surface created");
    
    // 空指针检查
    if (!component) {
        OHOS_LOGE(XCOMP_TAG, "component is NULL!");
        return;
    }
    if (!window) {
        OHOS_LOGE(XCOMP_TAG, "window is NULL!");
        return;
    }
    
    OHOS_LOGI(XCOMP_TAG, "component=%{public}p, window=%{public}p", component, window);
    
    std::lock_guard<std::mutex> lock(g_surfaceMutex);
    g_nativeWindow = window;
    
    OHOS_LOGI(XCOMP_TAG, "g_glContext=%{public}p", g_glContext);
    
    if (g_glContext) {
        g_glContext->SetNativeWindow(window);
        OHOS_LOGI(XCOMP_TAG, "SetNativeWindow done");
    } else {
        OHOS_LOGE(XCOMP_TAG, "g_glContext is NULL! Creating new one...");
        g_glContext = new OhosGLContext();
        g_glContext->SetNativeWindow(window);
    }
    
    // 获取表面尺寸
    OHOS_LOGI(XCOMP_TAG, "Getting surface size...");
    uint64_t width = 0, height = 0;
    int32_t ret = OH_NativeXComponent_GetXComponentSize(component, window, &width, &height);
    if (ret != 0) {
        OHOS_LOGE(XCOMP_TAG, "GetXComponentSize failed: %{public}d", ret);
        // 使用默认尺寸
        width = 1080;
        height = 1920;
    }
    g_surfaceWidth = static_cast<int>(width);
    g_surfaceHeight = static_cast<int>(height);
    
    OHOS_LOGI(XCOMP_TAG, "Surface size: %{public}dx%{public}d", g_surfaceWidth, g_surfaceHeight);
    INFO_LOG(Log::G3D, "Surface size: %dx%d", g_surfaceWidth, g_surfaceHeight);
    
    // 检查尺寸有效性
    if (g_surfaceWidth <= 0 || g_surfaceHeight <= 0) {
        OHOS_LOGE(XCOMP_TAG, "Invalid surface size!");
        return;
    }
    
    // 设置显示参数 - 这是 UI 渲染的关键！
    int dpi = 360;
    float dpi_scale_x = 240.0f / (float)dpi;
    float dpi_scale_y = 240.0f / (float)dpi;
    
    OHOS_LOGI(XCOMP_TAG, "Setting display params: dpi=%{public}d, scale=%{public}.2f", dpi, dpi_scale_x);
    
    // 调用 g_display.Recalculate 设置显示参数
    OHOS_LOGI(XCOMP_TAG, "Getting UIScaleFactorToMultiplier...");
    float uiScaleMultiplier = UIScaleFactorToMultiplier(g_Config.iUIScaleFactor);
    OHOS_LOGI(XCOMP_TAG, "uiScaleMultiplier=%{public}.2f", uiScaleMultiplier);
    
    OHOS_LOGI(XCOMP_TAG, "Calling g_display.Recalculate...");
    bool sizeChanged = g_display.Recalculate(g_surfaceWidth, g_surfaceHeight, dpi_scale_x, dpi_scale_y, uiScaleMultiplier);
    OHOS_LOGI(XCOMP_TAG, "Recalculate done, sizeChanged=%{public}d", sizeChanged);
    
    OHOS_LOGI(XCOMP_TAG, "Display: pixel=%{public}dx%{public}d, dp=%{public}dx%{public}d", 
              g_display.pixel_xres, g_display.pixel_yres,
              g_display.dp_xres, g_display.dp_yres);
    OHOS_LOGI(XCOMP_TAG, "Display scales: dpi_scale=(%{public}.3f,%{public}.3f), pixel_in_dps=(%{public}.3f,%{public}.3f)",
              g_display.dpi_scale_x, g_display.dpi_scale_y,
              g_display.pixel_in_dps_x, g_display.pixel_in_dps_y);
    
    // 调用 NativeResized - 这会初始化 UI 系统
    if (sizeChanged) {
        OHOS_LOGI(XCOMP_TAG, "Calling NativeResized...");
        NativeResized();
        OHOS_LOGI(XCOMP_TAG, "NativeResized completed");
    }
    
    // 启动渲染循环
    if (!g_renderLoopRunning.load()) {
        OHOS_LOGI(XCOMP_TAG, "Starting render loop...");
        RenderLoop();
        OHOS_LOGI(XCOMP_TAG, "RenderLoop() returned");
    } else {
        OHOS_LOGI(XCOMP_TAG, "Render loop already running");
    }
    
    OHOS_LOGI(XCOMP_TAG, "=== OnSurfaceCreated END ===");
}

void OnSurfaceChanged(OH_NativeXComponent* component, void* window) {
    INFO_LOG(Log::G3D, "XComponent surface changed");
    
    std::lock_guard<std::mutex> lock(g_surfaceMutex);
    
    // 获取新的表面尺寸
    uint64_t width = 0, height = 0;
    OH_NativeXComponent_GetXComponentSize(component, window, &width, &height);
    g_surfaceWidth = static_cast<int>(width);
    g_surfaceHeight = static_cast<int>(height);
    
    INFO_LOG(Log::G3D, "New surface size: %dx%d", g_surfaceWidth, g_surfaceHeight);
    
    if (g_glContext) {
        g_glContext->OnSurfaceChanged(g_surfaceWidth, g_surfaceHeight);
    }
}

void OnSurfaceDestroyed(OH_NativeXComponent* component, void* window) {
    INFO_LOG(Log::G3D, "XComponent surface destroyed");
    
    StopRenderLoop();
    
    std::lock_guard<std::mutex> lock(g_surfaceMutex);
    g_nativeWindow = nullptr;
}

void OnTouchEvent(OH_NativeXComponent* component, void* window) {
    // 获取触摸事件信息
    OH_NativeXComponent_TouchEvent touchEvent;
    int32_t ret = OH_NativeXComponent_GetTouchEvent(component, window, &touchEvent);
    if (ret != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        OHOS_LOGW(XCOMP_TAG, "Failed to get touch event: %{public}d", ret);
        return;
    }
    
    // UI 触摸震动：在 TOUCH_DOWN 时触发短震动（仅第一个触摸点）
    if (touchEvent.type == OH_NATIVEXCOMPONENT_DOWN && touchEvent.numPoints > 0 && g_Config.bHapticFeedback) {
        // 只为第一个触摸点震动，避免多点触摸时重复震动
        if (touchEvent.touchPoints[0].id == touchEvent.touchPoints[0].id) {
            OhosVibration::Vibrate(HAPTIC_VIRTUAL_KEY);
        }
    }
    
    // 处理每个触摸点
    for (uint32_t i = 0; i < touchEvent.numPoints && i < OH_MAX_TOUCH_POINTS_NUMBER; i++) {
        OH_NativeXComponent_TouchPoint touchPoint = touchEvent.touchPoints[i];
        
        TouchInput touch{};
        touch.id = touchPoint.id;
        
        // 转换坐标到 PPSSPP 的坐标系统
        // 参考 Android 实现：touch.x = x * display_scale_x * g_display.dpi_scale_x
        // 
        // display_scale_x = pixel_xres / display_xres (backbuffer 缩放)
        // 在 OHOS 中，Surface 大小就是 display 大小，所以：
        // display_scale_x = pixel_xres / surface_width
        // display_scale_y = pixel_yres / surface_height
        
        float display_scale_x = (float)g_display.pixel_xres / (float)g_surfaceWidth;
        float display_scale_y = (float)g_display.pixel_yres / (float)g_surfaceHeight;
        
        // Android 公式：x * display_scale_x * dpi_scale_x
        touch.x = touchPoint.x * display_scale_x * g_display.dpi_scale_x;
        touch.y = touchPoint.y * display_scale_y * g_display.dpi_scale_y;
        
        // 映射触摸动作
        switch (touchEvent.type) {
            case OH_NATIVEXCOMPONENT_DOWN:
                touch.flags = TOUCH_DOWN;
                break;
            case OH_NATIVEXCOMPONENT_UP:
                touch.flags = TOUCH_UP;
                break;
            case OH_NATIVEXCOMPONENT_MOVE:
                touch.flags = TOUCH_MOVE;
                break;
            case OH_NATIVEXCOMPONENT_CANCEL:
                touch.flags = TOUCH_CANCEL;
                break;
            default:
                continue;
        }
        
        touch.timestamp = time_now_d();
        touch.buttons = 0;  // 触摸屏没有按钮
        
        // 调试日志：输出触摸事件（仅在需要时启用）
        // if (touchEvent.type == OH_NATIVEXCOMPONENT_DOWN) {
        //     OHOS_LOGI(XCOMP_TAG, "Touch DOWN: (%{public}.1f,%{public}.1f) -> (%{public}.1f,%{public}.1f)", 
        //               touchPoint.x, touchPoint.y, touch.x, touch.y);
        // }
        
        // 发送触摸事件到 PPSSPP
        NativeTouch(touch);
    }
}

void RenderLoop() {
    if (g_renderLoopRunning.load()) {
        OHOS_LOGW(XCOMP_TAG, "Render loop already running");
        WARN_LOG(Log::G3D, "Render loop already running");
        return;
    }
    
    // 检查前置条件
    if (!g_glContext) {
        OHOS_LOGE(XCOMP_TAG, "Cannot start render loop: g_glContext is NULL");
        return;
    }
    if (!g_nativeWindow) {
        OHOS_LOGE(XCOMP_TAG, "Cannot start render loop: g_nativeWindow is NULL");
        return;
    }
    if (g_surfaceWidth <= 0 || g_surfaceHeight <= 0) {
        OHOS_LOGE(XCOMP_TAG, "Cannot start render loop: invalid surface size %{public}dx%{public}d", g_surfaceWidth, g_surfaceHeight);
        return;
    }
    
    OHOS_LOGI(XCOMP_TAG, "=== Starting Render Loop ===");
    OHOS_LOGI(XCOMP_TAG, "g_glContext=%{public}p, g_nativeWindow=%{public}p, size=%{public}dx%{public}d", 
              g_glContext, g_nativeWindow, g_surfaceWidth, g_surfaceHeight);
    INFO_LOG(Log::G3D, "Starting render loop");
    g_exitRenderLoop.store(false);
    
    // 分离线程，避免阻塞主线程
    g_renderThread = std::thread([]() {
        OHOS_LOGI(XCOMP_TAG, "=== Render Thread Started ===");
        OHOS_LOGI(XCOMP_TAG, "Surface: %{public}dx%{public}d", g_surfaceWidth, g_surfaceHeight);
        
        INFO_LOG(Log::G3D, "Render thread started");
        g_renderLoopRunning.store(true);
        
        // 再次检查（线程启动后可能状态已变）
        if (!g_glContext || !g_nativeWindow) {
            OHOS_LOGE(XCOMP_TAG, "Render thread: context or window became null!");
            g_renderLoopRunning.store(false);
            return;
        }
        
        // 初始化图形上下文
        OHOS_LOGI(XCOMP_TAG, "Initializing GL context from render thread...");
        if (!g_glContext->InitFromRenderThread(g_nativeWindow, g_surfaceWidth, g_surfaceHeight)) {
            OHOS_LOGE(XCOMP_TAG, "Failed to initialize graphics context!");
            ERROR_LOG(Log::G3D, "Failed to initialize graphics context");
            g_renderLoopRunning.store(false);
            return;
        }
        OHOS_LOGI(XCOMP_TAG, "GL context initialized successfully");
        
        // 初始化 PPSSPP 图形系统
        OHOS_LOGI(XCOMP_TAG, "Calling NativeInitGraphics...");
        if (!NativeInitGraphics(g_glContext)) {
            OHOS_LOGE(XCOMP_TAG, "NativeInitGraphics failed!");
            ERROR_LOG(Log::G3D, "Failed to initialize PPSSPP graphics");
            g_glContext->ShutdownFromRenderThread();
            g_renderLoopRunning.store(false);
            return;
        }
        OHOS_LOGI(XCOMP_TAG, "NativeInitGraphics succeeded");
        
        OHOS_LOGI(XCOMP_TAG, "Calling ThreadStart...");
        g_glContext->ThreadStart();
        g_rendererInited = true;
        OHOS_LOGI(XCOMP_TAG, "Renderer initialized, entering main loop");
        
        // 初始化帧率统计
        g_frameCount.store(0);
        g_lastFpsTime.store(time_now_d());
        g_lastFrameTime.store(time_now_d());
        
        INFO_LOG(Log::G3D, "Entering main render loop");
        
        // 主渲染循环
        int frameNum = 0;
        while (!g_exitRenderLoop.load()) {
            double frameStartTime = time_now_d();
            
            // 每 100 帧输出一次日志
//            if (frameNum % 100 == 0) {
//                OHOS_LOGI(XCOMP_TAG, "Frame %{public}d", frameNum);
//            }
            frameNum++;
            
            // 渲染一帧
            NativeFrame(g_glContext);
            
            // 处理渲染线程任务
            g_glContext->ThreadFrame(true);
            
            // 更新帧统计
            g_frameCount.fetch_add(1);
            double currentTime = time_now_d();
            double timeSinceLastFps = currentTime - g_lastFpsTime.load();
            
            // 每秒更新一次 FPS
            if (timeSinceLastFps >= 1.0) {
                int frameCount = g_frameCount.exchange(0);
                g_currentFps.store(frameCount / timeSinceLastFps);
                g_lastFpsTime.store(currentTime);
                
                // 每 5 秒输出一次 FPS 日志
                static double lastLogTime = 0.0;
                if (currentTime - lastLogTime >= 5.0) {
//                    OHOS_LOGI(XCOMP_TAG, "FPS: %.1{public}f", g_currentFps.load());
                    lastLogTime = currentTime;
                }
            }
            
            // 记录帧时间
            g_lastFrameTime.store(currentTime - frameStartTime);
            
            // 帧率控制：如果渲染太快，稍微休眠一下
            // 目标是 60 FPS (16.67ms per frame)
            double frameTime = time_now_d() - frameStartTime;
            const double targetFrameTime = 1.0 / 60.0;  // 60 FPS
            if (frameTime < targetFrameTime) {
                double sleepTime = targetFrameTime - frameTime;
                // 只在帧时间明显小于目标时才休眠
                if (sleepTime > 0.001) {  // 1ms
                    std::this_thread::sleep_for(
                        std::chrono::microseconds(static_cast<int>(sleepTime * 1000000.0))
                    );
                }
            }
        }
        
        INFO_LOG(Log::G3D, "Exiting render loop");
        OHOS_LOGI(XCOMP_TAG, "Exiting render loop");
        
        // 清理
        NativeShutdownGraphics();
        g_rendererInited = false;
        g_glContext->ThreadEnd();
        g_glContext->ShutdownFromRenderThread();
        
        g_renderLoopRunning.store(false);
        INFO_LOG(Log::G3D, "Render thread ended");
        OHOS_LOGI(XCOMP_TAG, "Render thread ended");
    });
    
    // 分离线程，让它独立运行
    g_renderThread.detach();
    OHOS_LOGI(XCOMP_TAG, "Render thread detached");
}

void StopRenderLoop() {
    if (!g_renderLoopRunning.load()) {
        OHOS_LOGI(XCOMP_TAG, "Render loop not running, nothing to stop");
        return;
    }
    
    INFO_LOG(Log::G3D, "Stopping render loop");
    OHOS_LOGI(XCOMP_TAG, "Stopping render loop...");
    g_exitRenderLoop.store(true);
    
    // 等待渲染线程结束（最多等待 3 秒）
    int waitCount = 0;
    while (g_renderLoopRunning.load() && waitCount < 300) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waitCount++;
    }
    
    if (g_renderLoopRunning.load()) {
        OHOS_LOGW(XCOMP_TAG, "Render loop did not stop in time");
    } else {
        OHOS_LOGI(XCOMP_TAG, "Render loop stopped");
    }
    
    INFO_LOG(Log::G3D, "Render loop stopped");
}

double GetCurrentFPS() {
    return g_currentFps.load();
}

double GetLastFrameTime() {
    return g_lastFrameTime.load();
}

int GetSurfaceWidth() {
    return g_surfaceWidth;
}

int GetSurfaceHeight() {
    return g_surfaceHeight;
}

} // namespace OhosXComponent
