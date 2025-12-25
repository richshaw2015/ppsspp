/**
 * 鸿蒙 OpenGL ES 上下文实现
 * 参考 android/jni/AndroidJavaGLContext.cpp
 */

#include "ohos_gl_context.h"
#include "Common/System/Display.h"
#include "Common/GPU/OpenGL/GLFeatures.h"
#include "Common/Log.h"
#include "Common/GPU/thin3d_create.h"
#include "Core/Config.h"
#include "Core/ConfigValues.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <native_window/external_window.h>
#include <native_buffer/native_buffer.h>

OhosGLContext::OhosGLContext()
    : display_(EGL_NO_DISPLAY)
    , surface_(EGL_NO_SURFACE)
    , context_(EGL_NO_CONTEXT)
    , config_(nullptr)
    , native_window_(nullptr)
    , width_(0)
    , height_(0)
    , initialized_(false)
    , draw_(nullptr)
    , renderManager_(nullptr)
    , state_(GraphicsContextState::PENDING) {
    SetGPUBackend(GPUBackend::OPENGL);
    INFO_LOG(Log::G3D, "OhosGLContext created");
}

OhosGLContext::~OhosGLContext() {
    Shutdown();
    if (draw_) {
        delete draw_;
        draw_ = nullptr;
    }
}

bool OhosGLContext::Init() {
    if (initialized_) {
        WARN_LOG(Log::G3D, "GL context already initialized");
        return true;
    }
    
    INFO_LOG(Log::G3D, "Initializing OpenGL ES context");
    
    if (!InitEGL()) {
        ERROR_LOG(Log::G3D, "Failed to initialize EGL");
        return false;
    }
    
    initialized_ = true;
    INFO_LOG(Log::G3D, "OpenGL ES context initialized successfully");
    
    return true;
}

void OhosGLContext::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    INFO_LOG(Log::G3D, "Shutting down OpenGL ES context");
    ShutdownEGL();
    initialized_ = false;
}

void OhosGLContext::SwapBuffers() {
    if (!initialized_ || surface_ == EGL_NO_SURFACE) {
        WARN_LOG(Log::G3D, "SwapBuffers called but not ready: initialized=%d, surface=%p", 
                 initialized_, (void*)surface_);
        return;
    }
    
    EGLBoolean result = eglSwapBuffers(display_, surface_);
    if (result != EGL_TRUE) {
        EGLint error = eglGetError();
        ERROR_LOG(Log::G3D, "eglSwapBuffers failed in SwapBuffers(): 0x%x", error);
    }
}

void OhosGLContext::SetNativeWindow(void* window) {
    native_window_ = window;
}

void OhosGLContext::OnSurfaceChanged(int width, int height) {
    width_ = width;
    height_ = height;
    INFO_LOG(Log::G3D, "Surface changed: %dx%d", width, height);
}

bool OhosGLContext::CheckGLExtensions() {
    // 调用全局的 CheckGLExtensions() 函数来初始化 gl_extensions 结构体
    // 这是必须的，因为整个 OpenGL 渲染系统都依赖这个结构体
    return ::CheckGLExtensions();
}

bool OhosGLContext::InitFromRenderThread(void* window, int desiredWidth, int desiredHeight) {
    INFO_LOG(Log::G3D, "OhosGLContext::InitFromRenderThread: desiredWidth=%d desiredHeight=%d", desiredWidth, desiredHeight);
    
    native_window_ = window;
    width_ = desiredWidth;
    height_ = desiredHeight;
    
    if (!InitEGL()) {
        ERROR_LOG(Log::G3D, "Failed to initialize EGL");
        state_ = GraphicsContextState::FAILED_INIT;
        return false;
    }
    
    // 确保 EGL 上下文完全就绪
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        ERROR_LOG(Log::G3D, "GL context not ready, error: 0x%x", err);
        state_ = GraphicsContextState::FAILED_INIT;
        return false;
    }
    
    // 检查 GL 扩展 - 这会调用 gl3stubInit()
    if (!CheckGLExtensions()) {
        ERROR_LOG(Log::G3D, "CheckGLExtensions failed");
        state_ = GraphicsContextState::FAILED_INIT;
        return false;
    }
    
    // OpenGL 在驱动中处理旋转渲染
    g_display.rotation = DisplayRotation::ROTATE_0;
    g_display.rot_matrix.setIdentity();
    
    // 创建 Draw 上下文
    draw_ = Draw::T3DCreateGLContext(false);
    if (!draw_) {
        ERROR_LOG(Log::G3D, "Failed to create Draw context");
        state_ = GraphicsContextState::FAILED_INIT;
        return false;
    }
    
    renderManager_ = (GLRenderManager*)draw_->GetNativeObject(Draw::NativeObject::RENDER_MANAGER);
    if (!renderManager_) {
        ERROR_LOG(Log::G3D, "Failed to get render manager");
        state_ = GraphicsContextState::FAILED_INIT;
        return false;
    }
    
    INFO_LOG(Log::G3D, "Setting inflight frames to %d", g_Config.iInflightFrames);
    renderManager_->SetInflightFrames(g_Config.iInflightFrames);
    
    // 设置 swap 函数 - 这是关键！没有这个，渲染的内容不会显示到屏幕上
    static int swapCount = 0;
    renderManager_->SetSwapFunction([this]() {
        swapCount++;
        if (swapCount <= 10 || swapCount % 60 == 0) {
            INFO_LOG(Log::G3D, "SwapFunction called (count=%d), display=%p, surface=%p", 
                     swapCount, (void*)display_, (void*)surface_);
        }
        if (display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE) {
            // 在 swap 之前确保所有 GL 命令都已执行完成
            glFinish();
            
            EGLBoolean result = eglSwapBuffers(display_, surface_);
            if (result != EGL_TRUE) {
                EGLint error = eglGetError();
                ERROR_LOG(Log::G3D, "eglSwapBuffers failed: 0x%x", error);
                
                // 尝试诊断错误
                if (error == EGL_BAD_SURFACE) {
                    ERROR_LOG(Log::G3D, "EGL_BAD_SURFACE: Surface may have been destroyed");
                } else if (error == EGL_BAD_NATIVE_WINDOW) {
                    ERROR_LOG(Log::G3D, "EGL_BAD_NATIVE_WINDOW: Native window may be invalid");
                } else if (error == EGL_CONTEXT_LOST) {
                    ERROR_LOG(Log::G3D, "EGL_CONTEXT_LOST: Context was lost, need to recreate");
                }
            } else if (swapCount <= 10) {
                INFO_LOG(Log::G3D, "eglSwapBuffers succeeded (count=%d)", swapCount);
            }
        } else {
            ERROR_LOG(Log::G3D, "SwapFunction called but display or surface is invalid!");
        }
    });
    INFO_LOG(Log::G3D, "SwapFunction set");
    
    if (!draw_->CreatePresets()) {
        ERROR_LOG(Log::G3D, "Failed to compile preset shaders");
        state_ = GraphicsContextState::FAILED_INIT;
        return false;
    }
    
    state_ = GraphicsContextState::INITIALIZED;
    INFO_LOG(Log::G3D, "OhosGLContext initialized successfully");
    return true;
}

void OhosGLContext::ShutdownFromRenderThread() {
    INFO_LOG(Log::G3D, "OhosGLContext::ShutdownFromRenderThread");
    
    renderManager_ = nullptr;  // owned by draw_
    if (draw_) {
        delete draw_;
        draw_ = nullptr;
    }
    
    ShutdownEGL();
    state_ = GraphicsContextState::SHUTDOWN;
}

void OhosGLContext::ThreadStart() {
    INFO_LOG(Log::G3D, "OhosGLContext::ThreadStart called");
    if (renderManager_) {
        // 必须先调用 StartThread 设置 runCompileThread_ = true
        // 否则 ThreadFrame 会因为断言失败而崩溃
        INFO_LOG(Log::G3D, "Calling renderManager_->StartThread()...");
        renderManager_->StartThread();
        INFO_LOG(Log::G3D, "Calling renderManager_->ThreadStart(draw_)...");
        renderManager_->ThreadStart(draw_);
        INFO_LOG(Log::G3D, "OhosGLContext::ThreadStart completed");
    } else {
        ERROR_LOG(Log::G3D, "OhosGLContext::ThreadStart: renderManager_ is null!");
    }
}

bool OhosGLContext::ThreadFrame(bool waitIfEmpty) {
    if (renderManager_) {
        return renderManager_->ThreadFrame(waitIfEmpty);
    }
    WARN_LOG(Log::G3D, "OhosGLContext::ThreadFrame: renderManager_ is null!");
    return true;
}

void OhosGLContext::ThreadEnd() {
    if (renderManager_) {
        renderManager_->ThreadEnd();
    }
}

void OhosGLContext::Poll() {
    // 在单线程模式下，我们需要定期处理 GL 队列
    // 使用 waitIfEmpty=false，这样如果队列为空就立即返回
    if (renderManager_) {
        // 处理队列中的任务，但不等待
        // 这可以防止队列满导致的死锁
        while (renderManager_->ThreadFrame(false)) {
            // 继续处理直到队列为空或完成一个 PRESENT
        }
    }
}

bool OhosGLContext::InitEGL() {
    // 1. 获取 EGL Display
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display_ == EGL_NO_DISPLAY) {
        ERROR_LOG(Log::G3D, "Failed to get EGL display");
        return false;
    }
    
    // 2. 初始化 EGL
    EGLint major, minor;
    if (!eglInitialize(display_, &major, &minor)) {
        EGLint error = eglGetError();
        ERROR_LOG(Log::G3D, "Failed to initialize EGL, error: 0x%x", error);
        return false;
    }
    INFO_LOG(Log::G3D, "EGL version: %d.%d", major, minor);
    
    // 3. 选择配置
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    
    EGLint numConfigs;
    if (!eglChooseConfig(display_, configAttribs, &config_, 1, &numConfigs) || numConfigs == 0) {
        EGLint error = eglGetError();
        ERROR_LOG(Log::G3D, "Failed to choose EGL config, error: 0x%x, numConfigs: %d", error, numConfigs);
        return false;
    }
    INFO_LOG(Log::G3D, "EGL config chosen, numConfigs: %d", numConfigs);
    
    // 4. 创建 EGL Surface
    if (native_window_) {
        // 在鸿蒙平台，EGLNativeWindowType 是 OHNativeWindow* 指针类型
        OHNativeWindow* nativeWindow = reinterpret_cast<OHNativeWindow*>(native_window_);
        
        // 设置 native window 的缓冲区格式和大小
        // 这对于 OpenGL 渲染正确显示非常重要
        int32_t code = SET_BUFFER_GEOMETRY;
        int32_t ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, code, width_, height_);
        if (ret != 0) {
            WARN_LOG(Log::G3D, "OH_NativeWindow_NativeWindowHandleOpt SET_BUFFER_GEOMETRY failed: %d", ret);
        } else {
            INFO_LOG(Log::G3D, "Native window buffer geometry set to %dx%d", width_, height_);
        }
        
        // 设置缓冲区格式为 RGBA8888
        code = SET_FORMAT;
        ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, code, NATIVEBUFFER_PIXEL_FMT_RGBA_8888);
        if (ret != 0) {
            WARN_LOG(Log::G3D, "OH_NativeWindow_NativeWindowHandleOpt SET_FORMAT failed: %d", ret);
        } else {
            INFO_LOG(Log::G3D, "Native window buffer format set to RGBA8888");
        }
        
        // 设置缓冲区使用标志 - GPU 渲染需要这个
        // 使用 HarmonyOS 定义的 usage 标志
#ifdef NATIVEBUFFER_USAGE_HW_RENDER
        code = SET_USAGE;
        uint64_t usage = NATIVEBUFFER_USAGE_HW_RENDER | NATIVEBUFFER_USAGE_HW_TEXTURE | NATIVEBUFFER_USAGE_MEM_DMA;
        ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, code, usage);
        if (ret != 0) {
            WARN_LOG(Log::G3D, "OH_NativeWindow_NativeWindowHandleOpt SET_USAGE failed: %d", ret);
        } else {
            INFO_LOG(Log::G3D, "Native window buffer usage set for GPU rendering");
        }
#else
        // 如果没有定义 usage 标志，尝试使用数值
        // NATIVEBUFFER_USAGE_HW_RENDER = 0x100, NATIVEBUFFER_USAGE_HW_TEXTURE = 0x200, NATIVEBUFFER_USAGE_MEM_DMA = 0x8
        code = SET_USAGE;
        uint64_t usage = 0x100 | 0x200 | 0x8;  // HW_RENDER | HW_TEXTURE | MEM_DMA
        ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, code, usage);
        if (ret != 0) {
            WARN_LOG(Log::G3D, "OH_NativeWindow_NativeWindowHandleOpt SET_USAGE failed: %d (using hardcoded values)", ret);
        } else {
            INFO_LOG(Log::G3D, "Native window buffer usage set for GPU rendering (using hardcoded values)");
        }
#endif
        
        // 查询当前格式以验证设置
        int32_t currentFormat = 0;
        code = GET_FORMAT;
        ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, code, &currentFormat);
        if (ret == 0) {
            INFO_LOG(Log::G3D, "Native window current format: %d", currentFormat);
        }
        
        surface_ = eglCreateWindowSurface(display_, config_, 
                                         reinterpret_cast<EGLNativeWindowType>(native_window_), 
                                         nullptr);
        if (surface_ == EGL_NO_SURFACE) {
            EGLint error = eglGetError();
            ERROR_LOG(Log::G3D, "Failed to create EGL surface, error: 0x%x", error);
            return false;
        }
        
        // 查询 EGL surface 的实际大小
        EGLint surfaceWidth = 0, surfaceHeight = 0;
        eglQuerySurface(display_, surface_, EGL_WIDTH, &surfaceWidth);
        eglQuerySurface(display_, surface_, EGL_HEIGHT, &surfaceHeight);
        INFO_LOG(Log::G3D, "EGL surface created successfully, actual size: %dx%d (requested: %dx%d)", 
                 surfaceWidth, surfaceHeight, width_, height_);
    }
    
    // 5. 创建 EGL Context
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    
    context_ = eglCreateContext(display_, config_, EGL_NO_CONTEXT, contextAttribs);
    if (context_ == EGL_NO_CONTEXT) {
        EGLint error = eglGetError();
        ERROR_LOG(Log::G3D, "Failed to create EGL context, error: 0x%x", error);
        return false;
    }
    INFO_LOG(Log::G3D, "EGL context created successfully");
    
    // 6. 绑定上下文
    if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
        EGLint error = eglGetError();
        ERROR_LOG(Log::G3D, "Failed to make EGL context current, error: 0x%x", error);
        return false;
    }
    INFO_LOG(Log::G3D, "EGL context made current successfully");
    
    // 7. 验证上下文
    EGLContext currentContext = eglGetCurrentContext();
    if (currentContext != context_) {
        ERROR_LOG(Log::G3D, "EGL context verification failed");
        return false;
    }
    
    // 8. 测试 OpenGL 渲染 - 绘制一个红色帧来验证渲染管线
    INFO_LOG(Log::G3D, "Testing OpenGL rendering with red clear...");
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);  // 红色
    glClear(GL_COLOR_BUFFER_BIT);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        ERROR_LOG(Log::G3D, "glClear failed with error: 0x%x", err);
    }
    
    // 立即 swap 来显示红色帧
    EGLBoolean swapResult = eglSwapBuffers(display_, surface_);
    if (swapResult != EGL_TRUE) {
        EGLint swapError = eglGetError();
        ERROR_LOG(Log::G3D, "Test eglSwapBuffers failed: 0x%x", swapError);
    } else {
        INFO_LOG(Log::G3D, "Test eglSwapBuffers succeeded - you should see a red screen briefly");
    }
    
    return true;
}

void OhosGLContext::ShutdownEGL() {
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}
