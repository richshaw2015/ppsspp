# OHOS 平台崩溃修复总结

本文档总结了 PPSSPP 在 OHOS 平台上遇到的两个主要崩溃问题及其修复方案。

## 问题 1：帧转储测试崩溃

### 症状
- 进入"工具 → 开发者工具 → 测试 → 帧转储测试"时崩溃
- 错误：`SIGSEGV` 空指针解引用

### 原因
多处代码直接调用 `gpuDebug->GetRecorder()` 而没有检查返回值

### 修复
在 5 个文件中添加了空指针检查：
1. `UI/DevScreens.cpp`
2. `Core/HLE/sceDisplay.cpp`
3. `Core/Debugger/WebSocket/GPURecordSubscriber.cpp`
4. `GPU/Debugger/Playback.cpp`
5. `ohos/entry/src/main/cpp/ohos_system_properties.cpp`

### 详细文档
- [FRAMEDUMP_CRASH_FIX.md](./FRAMEDUMP_CRASH_FIX.md)
- [FRAMEDUMP_CRASH_FIX_SUMMARY.md](./FRAMEDUMP_CRASH_FIX_SUMMARY.md)

---

## 问题 2：OpenGL 初始化崩溃 ⚠️ **当前问题**

### 症状
```
Signal:SIGSEGV(SEGV_ACCERR)@0x0000005b4c767520
#00 gl3stubInit+36
#01 CheckGLExtensions()+3000
#02 Draw::OpenGLContext::CreateFramebuffer
```

### 原因
`OhosGLContext::CheckGLExtensions()` 实现错误，没有调用全局的 `::CheckGLExtensions()` 函数，导致 OpenGL ES 3.0 函数指针未被初始化。

### 修复
修改 `ohos/entry/src/main/cpp/ohos_gl_context.cpp`：

```cpp
bool OhosGLContext::CheckGLExtensions() {
    // 调用全局的 CheckGLExtensions 函数来正确初始化 OpenGL 扩展
    return ::CheckGLExtensions();
}
```

### 详细文档
- [GL3STUB_CRASH_FIX.md](./GL3STUB_CRASH_FIX.md)

---

## 修复文件列表

### 帧转储崩溃修复
1. `UI/DevScreens.cpp`
2. `Core/HLE/sceDisplay.cpp`
3. `Core/Debugger/WebSocket/GPURecordSubscriber.cpp`
4. `GPU/Debugger/Playback.cpp`
5. `ohos/entry/src/main/cpp/ohos_system_properties.cpp`

### OpenGL 初始化崩溃修复
1. `ohos/entry/src/main/cpp/ohos_gl_context.cpp`
2. `Common/GPU/OpenGL/gl3stub.c` - **关键修复：跳过 eglGetProcAddress 动态加载**

---

## 测试建议

### 测试 1：OpenGL 初始化
1. 启动应用
2. 观察是否能成功初始化 OpenGL 上下文
3. 检查日志中是否有 "OhosGLContext initialized successfully"

### 测试 2：游戏加载
1. 选择并加载一个游戏
2. 观察是否能正常显示游戏画面
3. 检查是否有渲染错误

### 测试 3：帧转储功能
1. 在游戏运行时，进入"工具 → 开发者工具 → 测试"
2. 尝试"创建帧转储"功能
3. 应显示错误提示而不是崩溃

### 测试 4：帧转储测试
1. 进入"工具 → 开发者工具 → 测试 → 帧转储测试"
2. 选择一个测试文件
3. 应能正常下载和加载

---

## 调试技巧

### 查看崩溃日志
```bash
hdc shell hilog -x | grep -i "ppsspp\|crash\|sigsegv"
```

### 查看 OpenGL 日志
```bash
hdc shell hilog -x | grep -i "gl\|egl\|opengl"
```

### 查看应用日志
```bash
hdc shell hilog -x | grep "app.superedu.psp"
```

---

## 已知限制

1. **System_ShowFileInFolder** - 暂时禁用，需要完善实现
2. **OpenGL ES 版本** - 需要设备支持 OpenGL ES 3.0
3. **帧转储创建** - 需要游戏正在运行且 GPU 调试接口可用

---

## 后续工作

### 短期
1. ✅ 修复 OpenGL 初始化崩溃
2. ⏳ 测试游戏渲染是否正常
3. ⏳ 验证帧转储功能

### 中期
1. 完善 `System_ShowFileInFolder` 实现
2. 添加更详细的错误日志
3. 优化 OpenGL 性能

### 长期
1. 支持 Vulkan 渲染后端
2. 实现所有 OHOS 平台特性
3. 性能优化和稳定性改进

---

## 参考资料

- [OHOS OpenGL 实现](./OPENGL_IMPLEMENTATION.md)
- [OHOS 适配状态](./OHOS_ADAPTATION_STATUS.md)
- [渲染循环文档](./RENDER_LOOP.md)
