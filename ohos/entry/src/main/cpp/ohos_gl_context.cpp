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
        return;
    }
    
    eglSwapBuffers(display_, surface_);
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
    // 检查 OpenGL ES 扩展
    // 参考 Android 实现，这里简化处理
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    if (!extensions) {
        ERROR_LOG(Log::G3D, "Failed to get GL extensions");
        return false;
    }
    
    INFO_LOG(Log::G3D, "GL Extensions: %s", extensions);
    return true;
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
    
    renderManager_->SetInflightFrames(g_Config.iInflightFrames);
    
    // 设置 swap 函数 - 这是关键！没有这个，渲染的内容不会显示到屏幕上
    renderManager_->SetSwapFunction([this]() {
        if (display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE) {
            eglSwapBuffers(display_, surface_);
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
    if (renderManager_) {
        renderManager_->ThreadStart(draw_);
    }
}

bool OhosGLContext::ThreadFrame(bool waitIfEmpty) {
    if (renderManager_) {
        return renderManager_->ThreadFrame(waitIfEmpty);
    }
    return true;
}

void OhosGLContext::ThreadEnd() {
    if (renderManager_) {
        renderManager_->ThreadEnd();
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
        ERROR_LOG(Log::G3D, "Failed to initialize EGL");
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
    if (!eglChooseConfig(display_, configAttribs, &config_, 1, &numConfigs)) {
        ERROR_LOG(Log::G3D, "Failed to choose EGL config");
        return false;
    }
    
    // 4. 创建 EGL Surface
    if (native_window_) {
        // 在鸿蒙平台，EGLNativeWindowType 是 OHNativeWindow* 指针类型
        surface_ = eglCreateWindowSurface(display_, config_, 
                                         reinterpret_cast<EGLNativeWindowType>(native_window_), 
                                         nullptr);
        if (surface_ == EGL_NO_SURFACE) {
            EGLint error = eglGetError();
            ERROR_LOG(Log::G3D, "Failed to create EGL surface, error: 0x%x", error);
            return false;
        }
    }
    
    // 5. 创建 EGL Context
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    
    context_ = eglCreateContext(display_, config_, EGL_NO_CONTEXT, contextAttribs);
    if (context_ == EGL_NO_CONTEXT) {
        ERROR_LOG(Log::G3D, "Failed to create EGL context");
        return false;
    }
    
    // 6. 绑定上下文
    if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
        ERROR_LOG(Log::G3D, "Failed to make EGL context current");
        return false;
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
