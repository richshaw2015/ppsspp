/**
 * 鸿蒙 OpenGL ES 上下文
 * 参考 android/jni/AndroidJavaGLContext.h
 */

#ifndef OHOS_GL_CONTEXT_H
#define OHOS_GL_CONTEXT_H

#include "ohos_graphics_context.h"
#include "Common/GPU/OpenGL/GLRenderManager.h"
#include "Common/GPU/thin3d.h"
#include <EGL/egl.h>

/**
 * OpenGL ES 上下文实现
 * 参考 AndroidJavaEGLGraphicsContext
 */
class OhosGLContext : public OhosGraphicsContext {
public:
    OhosGLContext();
    virtual ~OhosGLContext();
    
    // 实现基类接口
    bool Init() override;
    void Shutdown() override;
    void SwapBuffers() override;
    int GetWidth() const override { return width_; }
    int GetHeight() const override { return height_; }
    std::string GetAPIType() const override { return "OpenGL ES"; }
    
    // GraphicsContext 接口
    bool InitFromRenderThread(void* window, int desiredWidth, int desiredHeight) override;
    void ShutdownFromRenderThread() override;
    void ThreadStart();
    bool ThreadFrame(bool waitIfEmpty);
    void ThreadEnd();
    Draw::DrawContext* GetDrawContext() override { return draw_; }
    
    // 鸿蒙特定接口
    void SetNativeWindow(void* window);
    void OnSurfaceChanged(int width, int height);
    
    GraphicsContextState GetState() const override { return state_; }
    
private:
    bool InitEGL();
    void ShutdownEGL();
    bool CheckGLExtensions();
    
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLConfig config_;
    void* native_window_;
    int width_;
    int height_;
    bool initialized_;
    
    Draw::DrawContext* draw_;
    GLRenderManager* renderManager_;
    GraphicsContextState state_;
};

#endif // OHOS_GL_CONTEXT_H
