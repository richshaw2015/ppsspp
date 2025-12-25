# Black Screen Root Cause Analysis

## Crash Analysis

### Crash Stack Trace
```
Signal:SIGSEGV(SEGV_MAPERR)@0x0000000000000008
#00 FastVec<GLRRenderData>::push_uninitialized()+24
#01 GLRenderManager::Clear()+100
#02 Draw::OpenGLContext::Clear()+264
#03 NativeFrame()+1796
```

### Root Cause
Attempted to call `g_draw->Clear()` immediately after `BeginFrame()`, but `GLRenderManager` requires an active render step before any rendering commands can be issued.

The assertion in `GLRenderManager::Clear()` shows:
```cpp
_dbg_assert_(curRenderStep_ && curRenderStep_->stepType == GLRStepType::RENDER);
```

This means `curRenderStep_` was NULL or not a RENDER step, causing the crash when trying to push to `curRenderStep_->commands`.

## Real Problem: Why is UI Not Rendering?

The black screen issue is NOT about the rendering pipeline being broken. The crash proved that:
1. ✅ OpenGL context is created and current
2. ✅ `NativeFrame()` is being called
3. ✅ `BeginFrame()` works
4. ✅ Render thread is running

The problem is that **UI rendering is not producing any visible output**.

## Hypothesis: Missing Render Target Setup

### Theory
In OpenGL ES, rendering requires:
1. A framebuffer to render to (default framebuffer = screen)
2. Proper viewport setup
3. Render commands
4. SwapBuffers to display

The issue might be:
- Default framebuffer (FBO 0) not properly bound
- Viewport not set correctly
- Rendering to wrong framebuffer
- SwapBuffers not working

### Evidence from Code

#### OHOS Implementation
```cpp
// In ohos_gl_context.cpp InitFromRenderThread()
renderManager_->SetSwapFunction([this]() {
    if (display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE) {
        eglSwapBuffers(display_, surface_);
    }
});
```

This looks correct, but we need to verify:
1. Is the swap function actually being called?
2. Is `eglSwapBuffers()` succeeding?
3. Is the default framebuffer (FBO 0) bound during rendering?

## Next Steps

### 1. Add Logging to Verify Swap
Add logging in the swap function to see if it's being called:

```cpp
renderManager_->SetSwapFunction([this]() {
    static int swapCount = 0;
    if (display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE) {
        EGLBoolean result = eglSwapBuffers(display_, surface_);
        if (swapCount % 60 == 0) {
            INFO_LOG(Log::G3D, "eglSwapBuffers called, result=%d, count=%d", result, swapCount);
        }
        swapCount++;
    } else {
        ERROR_LOG(Log::G3D, "SwapBuffers: display or surface is NULL!");
    }
});
```

### 2. Check Framebuffer Binding
Add logging to see what framebuffer is bound:

```cpp
// In NativeFrame, after BeginFrame()
GLint currentFBO = 0;
glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
static int fboCheckCounter = 0;
if (fboCheckCounter % 100 == 0) {
    INFO_LOG(Log::System, "Current FBO: %d", currentFBO);
}
fboCheckCounter++;
```

### 3. Check Viewport
```cpp
GLint viewport[4];
glGetIntegerv(GL_VIEWPORT, viewport);
static int vpCheckCounter = 0;
if (vpCheckCounter % 100 == 0) {
    INFO_LOG(Log::System, "Viewport: x=%d y=%d w=%d h=%d", 
        viewport[0], viewport[1], viewport[2], viewport[3]);
}
vpCheckCounter++;
```

### 4. Verify EGL Surface Size
```cpp
// In InitEGL after creating surface
EGLint width = 0, height = 0;
eglQuerySurface(display_, surface_, EGL_WIDTH, &width);
eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);
INFO_LOG(Log::G3D, "EGL Surface size: %dx%d", width, height);
```

## Comparison with Android

### Android OpenGL Context Setup
Android uses `AndroidJavaGLContext` which:
1. Creates EGL context on Java side
2. Makes it current on render thread
3. Sets up swap function
4. Everything "just works"

### OHOS OpenGL Context Setup
OHOS uses `OhosGLContext` which:
1. Creates EGL context on native side
2. Makes it current on render thread
3. Sets up swap function
4. **Something is different**

### Key Difference to Investigate
Android might be doing something extra that OHOS is missing:
- Window surface format/attributes
- EGL config selection
- Context attributes
- Surface size handling

## Alternative Approach: Use Vulkan

If OpenGL continues to have issues, consider switching to Vulkan:
- Vulkan is better supported on HarmonyOS
- More explicit control over rendering
- Better debugging tools
- Modern API

To switch:
```cpp
// In ohos_app.cpp or configuration
g_Config.iGPUBackend = (int)GPUBackend::VULKAN;
```

## Summary

The black screen is NOT because:
- ❌ Rendering pipeline is broken (it's working)
- ❌ OpenGL context failed to create (it's created)
- ❌ Render loop not running (it's running at 60fps)

The black screen IS because:
- ✅ UI rendering commands are not producing visible output
- ✅ Either rendering to wrong target, or swap not working, or viewport wrong

Next action: Add detailed logging to track down exactly where the rendering output is going.
