# Black Screen Debug - Phase 2

## Current Status
- ✅ Memory initialization working
- ✅ Graphics context (OpenGL) initialized
- ✅ Render loop running at 60 FPS
- ✅ `NativeFrame()` being called continuously
- ❌ Still black screen - no visible UI

## Debug Logging Added

Added comprehensive debug logging to `UI/NativeApp.cpp::NativeFrame()` to track:

### 1. Basic State (every 100 frames)
```cpp
INFO_LOG(Log::System, "NativeFrame: graphicsContext=%p, g_screenManager=%p", 
    graphicsContext, g_screenManager);
if (g_screenManager) {
    INFO_LOG(Log::System, "NativeFrame: Current screen: %s", 
        g_screenManager->topScreen() ? g_screenManager->topScreen()->tag() : "NULL");
}
```

### 2. Display Parameters (every 100 frames)
```cpp
INFO_LOG(Log::System, "NativeFrame: Display params - dp_xres=%d, dp_yres=%d, pixel_xres=%d, pixel_yres=%d", 
    g_display.dp_xres, g_display.dp_yres, g_display.pixel_xres, g_display.pixel_yres);
```

### 3. Rendering Calls (every 100 frames)
```cpp
INFO_LOG(Log::System, "NativeFrame: About to call g_screenManager->render()");
// ... render call ...
INFO_LOG(Log::System, "NativeFrame: render() returned, flags=%d", (int)renderFlags);
```

### 4. Present Calls (every 100 frames)
```cpp
INFO_LOG(Log::System, "NativeFrame: About to call g_draw->Present(), mode=%d", 
    (int)g_frameTiming.PresentMode());
// ... present call ...
INFO_LOG(Log::System, "NativeFrame: Present() completed");
```

## What to Look For in Logs

After rebuilding and running, check for:

1. **Is g_screenManager null?**
   - If null: NativeInit didn't complete properly
   - If not null: Good, continue checking

2. **What is the current screen?**
   - Should be "LogoScreen" initially
   - If NULL: Screen wasn't set up properly
   - If something else: Check if it's the right screen

3. **Are display parameters valid?**
   - `dp_xres` and `dp_yres` should be > 0
   - `pixel_xres` and `pixel_yres` should match surface size
   - If any are 0: Display wasn't initialized properly

4. **Is render() being called?**
   - Should see "About to call g_screenManager->render()"
   - Should see "render() returned, flags=X"
   - If not seeing these: Something is wrong before rendering

5. **Is Present() being called?**
   - Should see "About to call g_draw->Present()"
   - Should see "Present() completed"
   - If not seeing these: Rendering pipeline is broken

## Possible Issues

### Issue 1: g_screenManager is null
**Cause**: NativeInit didn't complete or failed
**Solution**: Check NativeInit logs, ensure it completes successfully

### Issue 2: Current screen is NULL
**Cause**: Screen wasn't set in NativeInit
**Solution**: Check screen initialization in NativeInit (around line 749-767)

### Issue 3: Display parameters are 0
**Cause**: g_display.Recalculate() wasn't called or failed
**Solution**: Check OnSurfaceCreated in ohos_xcomponent.cpp, ensure Recalculate is called

### Issue 4: render() not being called
**Cause**: Code path is exiting early from NativeFrame
**Solution**: Check for early returns in NativeFrame before render() call

### Issue 5: Present() not being called
**Cause**: Code path is exiting after render() but before Present()
**Solution**: Check for early returns between render() and Present()

### Issue 6: Everything looks good but still black
**Cause**: Rendering commands not reaching GPU, or wrong framebuffer
**Solution**: 
- Check if OpenGL context is current on render thread
- Verify framebuffer binding
- Add test rendering (clear screen to red) to verify pipeline

## Next Steps

1. **Rebuild and run** with new debug logging
2. **Collect logs** showing the new debug output
3. **Analyze** which of the above issues is occurring
4. **Fix** the identified issue

## Test Rendering

If all checks pass but still black screen, add test rendering to verify pipeline:

```cpp
// In NativeFrame, before g_screenManager->render()
g_draw->Clear(Draw::ClearFlag::COLOR, 0xFF0000FF, 0.0f, 0); // Clear to red
```

If screen turns red, rendering pipeline works and issue is in UI rendering logic.
If still black, rendering pipeline itself is broken.
