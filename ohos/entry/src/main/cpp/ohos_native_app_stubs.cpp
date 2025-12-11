/**
 * PPSSPP NativeApp 函数的桩实现
 * 这些函数在 UI/NativeApp.cpp 中实现，这里提供最小的桩实现以通过链接
 * 后续需要集成完整的 UI 库
 */

#include "Common/GraphicsContext.h"
#include "Common/Log.h"

// 临时桩实现 - 后续需要集成完整的 UI 库
bool NativeInitGraphics(GraphicsContext *graphicsContext) {
    INFO_LOG(Log::System, "NativeInitGraphics (stub)");
    // TODO: 集成完整的 UI 库实现
    return true;
}

void NativeFrame(GraphicsContext *graphicsContext) {
    // TODO: 集成完整的 UI 库实现
    // 这里应该调用 UI 渲染和游戏逻辑更新
}

void NativeShutdownGraphics() {
    INFO_LOG(Log::System, "NativeShutdownGraphics (stub)");
    // TODO: 集成完整的 UI 库实现
}
