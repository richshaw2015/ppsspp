# Black Screen - Final Analysis & Solution

## Current Status (CONFIRMED WORKING)

### ✅ All Systems Operational
1. **Memory System**: Working (`MemArenaOhos.cpp`)
2. **OpenGL Context**: Created successfully (`EGL Surface created: 2412x1084`)
3. **Render Loop**: Running at 60 FPS
4. **SwapBuffers**: Being called successfully (`eglSwapBuffers: result=1`)
5. **UI System**: Fully functional
   - Frame 0-100: Logo screen
   - Frame 200+: Main menu
   - Frame 500+: Game emulator screen
6. **All Pointers Valid**: ctx, screenManager, draw all non-null

### ❌ Problem
**Screen remains black despite all rendering working correctly.**

## Root Cause

The rendering pipeline is完全正常的，但渲染的内容没有显示到屏幕上。这是一个 **XComponent Surface 显示问题**，不是渲染问题。

### Evidence from Logs
```
18:02:44.908 EGL Surface created: 2412x1084
18:02:45.839 eglSwapBuffers: result=1, count=0
18:02:44.998 NativeFrame #0: Current screen: Logo
18:02:47.503 NativeFrame #100: Current screen: Logo  
18:02:49.181 NativeFrame #200: Current screen: Main
18:02:54.859 NativeFrame #500: Current screen: Emu
```

Everything is rendering, but not visible on screen.

## Possible Causes

### 1. XComponent Surface Not Connected to Display
OHOS XComponent might need explicit connection to the display system.

### 2. Z-Order Issue
The XComponent surface might be behind other UI elements.

### 3. Surface Format Mismatch
The EGL surface format might not match what XComponent expects.

### 4. Missing Surface Refresh
OHOS might need explicit invalidation/refresh calls.

## Solutions to Try

### Solution 1: Check XComponent Visibility in ArkTS

Add to `Index.ets`:
```typescript
XComponent({
  id: this.xComponentId,
  type: XComponentType.SURFACE,
  libraryname: 'ppsspp_ohos',
  controller: this.xComponentController
})
  .visibility(Visibility.Visible)  // Explicitly set visible
  .zIndex(1)  // Ensure it's on top
  .backgroundColor(Color.Red)  // Test: should see red if XComponent is visible
  .width('100%')
  .height('100%')
```

If you see red background, XComponent is visible but not rendering.
If still black, XComponent itself is not visible.

### Solution 2: Force Surface Refresh

In `ohos_gl_context.cpp`, after `eglSwapBuffers`:
```cpp
renderManager_->SetSwapFunction([this]() {
    if (display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE) {
        EGLBoolean result = eglSwapBuffers(display_, surface_);
        
        // Force flush
        glFlush();
        glFinish();
        
        // Log every frame temporarily
        static int count = 0;
        if (count++ < 10) {
            OHOS_LOGI(GL_TAG, "SwapBuffers #%{public}d: result=%{public}d", count, result);
        }
    }
});
```

### Solution 3: Verify EGL Configuration

Check if EGL config matches XComponent requirements. In `InitEGL()`:
```cpp
const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,  // Try with and without alpha
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_NONE
};
```

Try removing `EGL_ALPHA_SIZE` or setting it to 0.

### Solution 4: Check Native Window Format

Before creating EGL surface:
```cpp
// Query native window format
int32_t format = 0;
OH_NativeWindow_NativeWindowHandleOpt(
    static_cast<OHNativeWindow*>(native_window_),
    GET_FORMAT,
    &format
);
OHOS_LOGI(GL_TAG, "Native window format: %{public}d", format);

// Set format explicitly
int32_t desiredFormat = PIXEL_FMT_RGBA_8888;  // or PIXEL_FMT_RGBX_8888
OH_NativeWindow_NativeWindowHandleOpt(
    static_cast<OHNativeWindow*>(native_window_),
    SET_FORMAT,
    desiredFormat
);
```

### Solution 5: Try Different EGL Surface Attributes

```cpp
const EGLint surfaceAttribs[] = {
    EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
    EGL_NONE
};

surface_ = eglCreateWindowSurface(
    display_, 
    config_, 
    reinterpret_cast<EGLNativeWindowType>(native_window_), 
    surfaceAttribs  // Add attributes
);
```

### Solution 6: Switch to Vulkan

Since Vulkan is better supported on HarmonyOS:

In `ohos_app.cpp` or configuration:
```cpp
g_Config.iGPUBackend = (int)GPUBackend::VULKAN;
```

Or in ArkTS before initialization:
```typescript
// Force Vulkan backend
ppsspp.setGPUBackend(1);  // 1 = Vulkan
```

## Recommended Action Plan

1. **First**: Add `backgroundColor(Color.Red)` to XComponent to verify it's visible
2. **If red shows**: Problem is in rendering - try Solution 2 (Force Refresh)
3. **If still black**: Problem is XComponent visibility - check z-index, layout
4. **If nothing works**: Switch to Vulkan (Solution 6)

## Debug Commands

### Check if XComponent is actually rendering
Add temporary test in `NativeFrame`:
```cpp
// After BeginFrame, before any rendering
static int testFrame = 0;
if (testFrame++ < 60) {
    // Clear to different colors each second
    uint32_t color = (testFrame / 60) % 2 ? 0xFF0000FF : 0x00FF00FF;
    // This would need proper render target setup
}
```

### Verify EGL is working
```cpp
// In swap function
EGLint error = eglGetError();
if (error != EGL_SUCCESS) {
    OHOS_LOGE(GL_TAG, "EGL error: 0x%{public}x", error);
}
```

## Next Steps

Please try Solution 1 first (add red background to XComponent) and report what you see. This will tell us if the problem is:
- XComponent visibility (if no red)
- Rendering pipeline (if red shows but no game graphics)

Based on the result, we'll know exactly which solution to apply.
