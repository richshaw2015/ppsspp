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
 * 鸿蒙 Vulkan 上下文
 * 参考 android/jni/AndroidVulkanContext.h
 */

#ifndef OHOS_VULKAN_CONTEXT_H
#define OHOS_VULKAN_CONTEXT_H

#include "ohos_graphics_context.h"
#include "Common/GPU/thin3d.h"

class VulkanContext;

/**
 * Vulkan 上下文实现
 * 参考 AndroidVulkanContext
 */
class OhosVulkanContext : public OhosGraphicsContext {
public:
    OhosVulkanContext();
    virtual ~OhosVulkanContext();
    
    // 初始化 Vulkan API（在主线程调用）
    bool InitAPI();
    
    // 实现基类接口
    bool Init() override { return InitAPI(); }
    void Shutdown() override;
    void SwapBuffers() override {}  // Vulkan 不需要手动 swap
    int GetWidth() const override;
    int GetHeight() const override;
    std::string GetAPIType() const override { return "Vulkan"; }
    
    // GraphicsContext 接口
    bool InitFromRenderThread(void* window, int desiredWidth, int desiredHeight) override;
    void ShutdownFromRenderThread() override;
    void Resize() override;
    
    Draw::DrawContext* GetDrawContext() override { return draw_; }
    void* GetAPIContext() { return g_Vulkan; }
    
    GraphicsContextState GetState() const override { return state_; }
    
private:
    VulkanContext *g_Vulkan = nullptr;
    Draw::DrawContext *draw_ = nullptr;
    GraphicsContextState state_ = GraphicsContextState::PENDING;
};

#endif // OHOS_VULKAN_CONTEXT_H
