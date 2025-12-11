/**
 * 鸿蒙图形上下文接口
 * 参考 android/jni/AndroidGraphicsContext.h
 */

#ifndef OHOS_GRAPHICS_CONTEXT_H
#define OHOS_GRAPHICS_CONTEXT_H

#include "Common/GraphicsContext.h"
#include "Common/GPU/thin3d.h"
#include "Core/ConfigValues.h"
#include <string>

enum class GraphicsContextState {
    PENDING,
    INITIALIZED,
    FAILED_INIT,
    SHUTDOWN,
};

/**
 * 图形上下文基类
 * 继承自 PPSSPP 的 GraphicsContext
 * 参考 AndroidGraphicsContext
 */
class OhosGraphicsContext : public GraphicsContext {
public:
    virtual ~OhosGraphicsContext() {}
    
    /**
     * 初始化图形上下文
     */
    virtual bool Init() = 0;
    
    /**
     * 从渲染线程初始化
     * 参考 AndroidGraphicsContext::InitFromRenderThread
     */
    virtual bool InitFromRenderThread(void* window, int desiredWidth, int desiredHeight) = 0;
    
    // 实现基类的 InitFromRenderThread
    bool InitFromRenderThread(std::string *errorMessage) override {
        // 鸿蒙版本使用带参数的版本，这个版本不使用
        return true;
    }
    
    /**
     * 从渲染线程关闭
     */
    void ShutdownFromRenderThread() override = 0;
    
    /**
     * 交换缓冲区
     */
    virtual void SwapBuffers() = 0;
    
    /**
     * 获取渲染宽度
     */
    virtual int GetWidth() const = 0;
    
    /**
     * 获取渲染高度
     */
    virtual int GetHeight() const = 0;
    
    /**
     * 获取图形 API 类型
     */
    virtual std::string GetAPIType() const = 0;
    
    /**
     * 获取状态
     */
    virtual GraphicsContextState GetState() const = 0;
    
    // 实现 GraphicsContext 接口
    void Resize() override {}
    void Shutdown() override = 0;

protected:
    void SetGPUBackend(GPUBackend backend) {
        gpuBackend_ = backend;
    }
    
    GPUBackend gpuBackend_ = GPUBackend::OPENGL;
};

#endif // OHOS_GRAPHICS_CONTEXT_H
