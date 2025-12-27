// PPSSPP - OpenHarmony Platform Adaptation
// Copyright (c) 2024-2025 richshaw
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License 2.0 for more details.
//
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

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
 * 
 * 注意：OHOS 使用单线程模式，与 Android 的双线程模式不同。
 * Android 有两个线程：
 * 1. EmuThread - 运行 NativeFrame，生成渲染命令
 * 2. AndroidRender (Java GL 线程) - 调用 ThreadFrame，执行渲染命令
 * 
 * OHOS 只有一个线程，所以需要在 NativeFrame 之后立即调用 ThreadFrame。
 * 但是 NativeFrame 内部的 CopyDisplayToOutput 会调用 FlushSync，
 * 这会等待 ThreadFrame 处理队列，导致死锁。
 * 
 * 解决方案：使用 Poll() 方法定期处理队列，避免死锁。
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
    
    // 单线程模式下的队列处理
    // 在 NativeFrame 期间定期调用，避免队列满导致死锁
    void Poll() override;
    
    GraphicsContextState GetState() const override { return state_; }
    
    GLRenderManager* GetRenderManager() { return renderManager_; }
    
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
