# Black Screen Fix Attempt

## Changes Made

### 1. Always Call NativeResized()
**File**: `ohos/entry/src/main/cpp/ohos_xcomponent.cpp`

**Problem**: `NativeResized()` was only called when `sizeChanged` was true. On first initialization, this might not be the case, causing UI system to not be properly initialized.

**Fix**: Always call `NativeResized()` after `g_display.Recalculate()`, regardless of `sizeChanged` value.

```cpp
// Before:
if (sizeChanged) {
    NativeResized();
}

// After:
// Always call on first initialization
NativeResized();
```

### 2. Add Test Rendering
**File**: `UI/NativeApp.cpp`

**Purpose**: Verify that the rendering pipeline is working by clearing the screen to red for the first 5 seconds.

**Implementation**: Added test code in `NativeFrame()` that clears the screen to red (0xFF0000FF) for the first 300 frames (~5 seconds at 60fps).

```cpp
// Test rendering - clear to red
static int testRenderCounter = 0;
if (testRenderCounter < 300) {
    g_draw->Clear(Draw::ClearFlag::COLOR, 0xFF0000FF, 0.0f, 0);  // Red
    testRenderCounter++;
}
```

## Expected Results

### If Screen Turns Red
✅ **Rendering pipeline is working!**
- OpenGL context is properly set up
- Framebuffer is correctly bound
- Present() is working
- **Problem is in UI rendering logic**

Next steps:
1. Remove test rendering code
2. Debug why UI is not rendering
3. Check if `g_screenManager` is properly initialized
4. Check if current screen is set correctly
5. Verify UI draw commands are being issued

### If Screen Stays Black
❌ **Rendering pipeline is broken**
- OpenGL context might not be current on render thread
- Framebuffer might not be bound correctly
- Present() might not be swapping buffers
- Window surface might not be properly connected

Next steps:
1. Check OpenGL context state
2. Verify EGL surface is created and current
3. Check if `eglSwapBuffers()` is being called
4. Add OpenGL error checking
5. Verify window surface is valid

## Comparison with Android

### Android Initialization Order
1. Create graphics context
2. Set display parameters (`g_display.Recalculate()`)
3. Call `NativeResized()`
4. Start render thread
5. In render thread:
   - `InitFromRenderThread()`
   - `NativeInitGraphics()`
   - `ThreadStart()`
   - Enter render loop with `NativeFrame()`

### OHOS Current Order
1. Create graphics context
2. On surface created (main thread):
   - Set display parameters (`g_display.Recalculate()`)
   - Call `NativeResized()` (NOW ALWAYS CALLED)
   - Start render thread
3. In render thread:
   - `InitFromRenderThread()`
   - `NativeInitGraphics()`
   - `ThreadStart()`
   - Enter render loop with `NativeFrame()`

The order is now similar to Android, with the key fix being that `NativeResized()` is always called.

## Testing Instructions

1. **Rebuild the project**
   ```bash
   cd ohos
   hvigorw clean
   hvigorw assembleHap
   ```

2. **Install and run**

3. **Check the screen**:
   - **Red screen**: Rendering works, UI rendering issue
   - **Black screen**: Rendering pipeline broken

4. **Check logs** for:
   ```
   "Calling NativeResized (sizeChanged=X)..."
   "NativeResized completed"
   "Test render: clearing to red, frame X"
   ```

## Rollback Instructions

If this doesn't help, revert these changes:

### Revert NativeResized change:
```cpp
// Restore conditional call
if (sizeChanged) {
    NativeResized();
}
```

### Remove test rendering:
```cpp
// Remove the test rendering block in NativeFrame()
```

## Next Debugging Steps

Based on test results:

### If Red Screen Appears
1. Remove test rendering code
2. Add logging to `g_screenManager->render()` to see what it's doing
3. Check if `g_screenManager->topScreen()` returns valid screen
4. Verify UI draw commands are being issued
5. Check if fonts/textures are loaded

### If Still Black
1. Add OpenGL error checking after each call
2. Verify EGL context is current: `eglGetCurrentContext()`
3. Check framebuffer status: `glCheckFramebufferStatus()`
4. Verify window surface: `eglQuerySurface()`
5. Add logging to `eglSwapBuffers()` to see if it's being called
6. Compare with working Android OpenGL setup

## Related Files
- `ohos/entry/src/main/cpp/ohos_xcomponent.cpp` - Surface and render loop management
- `ohos/entry/src/main/cpp/ohos_gl_context.cpp` - OpenGL context implementation
- `UI/NativeApp.cpp` - Main rendering loop
- `android/jni/app-android.cpp` - Android reference implementation
