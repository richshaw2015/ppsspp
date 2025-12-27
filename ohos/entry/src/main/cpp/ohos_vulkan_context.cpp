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
 * 鸿蒙 Vulkan 上下文实现
 * 参考 android/jni/AndroidVulkanContext.cpp
 */

#include "ohos_vulkan_context.h"
#include "Common/System/Display.h"
#include "Common/System/NativeApp.h"
#include "Common/System/System.h"
#include "Common/Log.h"
#include "Common/GPU/Vulkan/VulkanContext.h"
#include "Common/GPU/Vulkan/VulkanDebug.h"
#include "Common/GPU/Vulkan/VulkanLoader.h"
#include "Common/GPU/Vulkan/VulkanRenderManager.h"
#include "Common/GPU/thin3d_create.h"
#include "Core/Config.h"
#include "Core/ConfigValues.h"
#include "Core/System.h"
#include "GPU/Vulkan/VulkanUtil.h"

// OHOS 使用与 Android 相同的 ANativeWindow
#include <native_window/external_window.h>

OhosVulkanContext::OhosVulkanContext() {
    INFO_LOG(Log::G3D, "OhosVulkanContext created");
}

OhosVulkanContext::~OhosVulkanContext() {
    delete g_Vulkan;
    g_Vulkan = nullptr;
}

bool OhosVulkanContext::InitAPI() {
    INFO_LOG(Log::G3D, "OhosVulkanContext::InitAPI");
    init_glslang();

    g_LogOptions.breakOnError = true;
    g_LogOptions.breakOnWarning = true;
    g_LogOptions.msgBoxOnError = false;

    INFO_LOG(Log::G3D, "Creating Vulkan context");

    std::string errorStr;
    if (!VulkanLoad(&errorStr)) {
        ERROR_LOG(Log::G3D, "Failed to load Vulkan driver library: %s", errorStr.c_str());
        state_ = GraphicsContextState::FAILED_INIT;
        return false;
    }

    if (!g_Vulkan) {
        g_Vulkan = new VulkanContext();
    }

    VulkanContext::CreateInfo info{};
    InitVulkanCreateInfoFromConfig(&info);
    
    if (!g_Vulkan->CreateInstanceAndDevice(info)) {
        delete g_Vulkan;
        g_Vulkan = nullptr;
        state_ = GraphicsContextState::FAILED_INIT;
        return false;
    }

    INFO_LOG(Log::G3D, "Vulkan device created!");
    state_ = GraphicsContextState::INITIALIZED;
    return true;
}


bool OhosVulkanContext::InitFromRenderThread(void* window, int desiredWidth, int desiredHeight) {
    INFO_LOG(Log::G3D, "OhosVulkanContext::InitFromRenderThread: desiredWidth=%d desiredHeight=%d", 
             desiredWidth, desiredHeight);
    
    if (!g_Vulkan) {
        ERROR_LOG(Log::G3D, "OhosVulkanContext::InitFromRenderThread: No Vulkan context");
        return false;
    }

    // OHOS 使用与 Android 相同的 window system
    // OHNativeWindow 与 ANativeWindow 兼容
    VkResult res = g_Vulkan->InitSurface(WINDOWSYSTEM_ANDROID, window, nullptr);
    if (res != VK_SUCCESS) {
        ERROR_LOG(Log::G3D, "g_Vulkan->InitSurface failed: '%s'", VulkanResultToString(res));
        return false;
    }

    bool useMultiThreading = g_Config.bRenderMultiThreading;
    if (g_Config.iInflightFrames == 1) {
        useMultiThreading = false;
    }
    draw_ = Draw::T3DCreateVulkanContext(g_Vulkan, useMultiThreading);

    VkPresentModeKHR presentMode = ConfigPresentModeToVulkan(draw_);

    bool success = false;
    if (g_Vulkan->InitSwapchain(presentMode)) {
        SetGPUBackend(GPUBackend::VULKAN);
        success = draw_->CreatePresets();
        _assert_msg_(success, "Failed to compile preset shaders");
        draw_->HandleEvent(Draw::Event::GOT_BACKBUFFER, g_Vulkan->GetBackbufferWidth(), g_Vulkan->GetBackbufferHeight());

        VulkanRenderManager *renderManager = (VulkanRenderManager *)draw_->GetNativeObject(Draw::NativeObject::RENDER_MANAGER);
        renderManager->SetInflightFrames(g_Config.iInflightFrames);
        success = renderManager->HasBackbuffers();
    }

    INFO_LOG(Log::G3D, "OhosVulkanContext::InitFromRenderThread completed, %s", success ? "successfully" : "but failed");
    if (!success) {
        g_Vulkan->DestroySwapchain();
        g_Vulkan->DestroySurface();
        g_Vulkan->DestroyDevice();
        g_Vulkan->DestroyInstance();
    }
    return success;
}

void OhosVulkanContext::ShutdownFromRenderThread() {
    INFO_LOG(Log::G3D, "OhosVulkanContext::ShutdownFromRenderThread");
    if (draw_) {
        draw_->HandleEvent(Draw::Event::LOST_BACKBUFFER, g_Vulkan->GetBackbufferWidth(), g_Vulkan->GetBackbufferHeight());
        delete draw_;
        draw_ = nullptr;
    }
    if (g_Vulkan) {
        g_Vulkan->WaitUntilQueueIdle();
        g_Vulkan->PerformPendingDeletes();
        g_Vulkan->DestroySwapchain();
        g_Vulkan->DestroySurface();
    }
    INFO_LOG(Log::G3D, "Done with ShutdownFromRenderThread");
}

void OhosVulkanContext::Shutdown() {
    INFO_LOG(Log::G3D, "OhosVulkanContext::Shutdown");
    if (g_Vulkan) {
        g_Vulkan->DestroyDevice();
        g_Vulkan->DestroyInstance();
    }
    finalize_glslang();
    INFO_LOG(Log::G3D, "OhosVulkanContext::Shutdown completed");
}

void OhosVulkanContext::Resize() {
    if (!g_Vulkan || !draw_) {
        return;
    }
    
    INFO_LOG(Log::G3D, "OhosVulkanContext::Resize begin (oldsize: %dx%d)", 
             g_Vulkan->GetBackbufferWidth(), g_Vulkan->GetBackbufferHeight());
    
    draw_->HandleEvent(Draw::Event::LOST_BACKBUFFER, g_Vulkan->GetBackbufferWidth(), g_Vulkan->GetBackbufferHeight());
    g_Vulkan->DestroySwapchain();
    g_Vulkan->DestroySurface();
    g_Vulkan->ReinitSurface();

    VkPresentModeKHR presentMode = ConfigPresentModeToVulkan(draw_);
    g_Vulkan->InitSwapchain(presentMode);
    draw_->HandleEvent(Draw::Event::GOT_BACKBUFFER, g_Vulkan->GetBackbufferWidth(), g_Vulkan->GetBackbufferHeight());
    
    INFO_LOG(Log::G3D, "OhosVulkanContext::Resize end (final size: %dx%d)", 
             g_Vulkan->GetBackbufferWidth(), g_Vulkan->GetBackbufferHeight());
}

int OhosVulkanContext::GetWidth() const {
    if (g_Vulkan) {
        return g_Vulkan->GetBackbufferWidth();
    }
    return 0;
}

int OhosVulkanContext::GetHeight() const {
    if (g_Vulkan) {
        return g_Vulkan->GetBackbufferHeight();
    }
    return 0;
}
