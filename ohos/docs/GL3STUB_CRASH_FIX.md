# OpenGL ES 3.0 初始化崩溃修复

## 问题描述

在鸿蒙系统上运行游戏时，应用在启动后立即崩溃，崩溃堆栈显示：

```
Signal:SIGSEGV(SEGV_ACCERR)@0x0000005a2a667520
#00 pc 0000000001284e24 libppsspp_ohos.so(gl3stubInit+36)
#01 pc 0000000001287554 libppsspp_ohos.so(CheckGLExtensions()+3000)
#02 pc 0000000001298e58 libppsspp_ohos.so(Draw::OpenGLContext::CreateFramebuffer...)
```

## 根本原因

**鸿蒙系统直接链接 GLESv3 库，不需要动态加载 OpenGL ES 3.0 函数指针。**

1. **CMakeLists.txt 中已链接 GLESv3**
   ```cmake
   target_link_libraries(ppsspp_ohos PUBLIC GLESv3)
   ```
   这意味着所有 GL ES 3.0 函数（如 `glReadBuffer`、`glTexImage3D` 等）已经由系统库提供。

2. **gl3stubInit() 尝试覆盖系统函数**
   - `gl3stubInit()` 使用 `eglGetProcAddress()` 获取函数指针
   - 然后将这些指针存储到全局变量中
   - 在鸿蒙上，这些全局变量可能与系统库中的符号冲突
   - 导致写入只读内存区域，触发 `SEGV_ACCERR`

3. **崩溃地址一致**
   - 崩溃地址 `0x0000005a2a667520` 在多次崩溃中保持一致
   - 这表明是同一个内存位置的访问问题
   - 很可能是 `glReadBuffer` 等函数指针变量的地址

## 解决方案

### 1. 修改 GLCommon.h - 包含正确的头文件

```cpp
#elif PPSSPP_PLATFORM(OHOS)
// 鸿蒙系统直接使用 GLES3 头文件（支持 OpenGL ES 3.2）
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#define GL_BGRA_EXT 0x80E1
```

### 2. 修改 gl3stub.c 和 gl3stub.h - 排除 OHOS 平台

**gl3stub.c:**
```c
#if defined(USING_GLES2)
#if !PPSSPP_PLATFORM(IOS) && !PPSSPP_PLATFORM(OHOS)
// 鸿蒙系统直接链接 GLESv3 库，不需要动态加载函数指针
#include "EGL/egl.h"

GLboolean gl3stubInit() {
    // ... 动态加载代码 ...
}

// ... 函数指针定义 ...

#endif // !PPSSPP_PLATFORM(IOS) && !PPSSPP_PLATFORM(OHOS)

#if PPSSPP_PLATFORM(IOS) || PPSSPP_PLATFORM(OHOS)
// iOS 和 OHOS 平台不需要动态加载 GL ES 3.0 函数
GLboolean gl3stubInit() {
    return GL_TRUE;
}
#endif
#endif // GLES2
```

**gl3stub.h:**
```c
#if !PPSSPP_PLATFORM(IOS) && !PPSSPP_PLATFORM(OHOS)
// iOS 和 OHOS 平台不需要这些声明
// 函数已在系统头文件中声明

// ... 类型定义、常量和函数指针声明 ...

#endif // !PPSSPP_PLATFORM(IOS) && !PPSSPP_PLATFORM(OHOS)
```

### 3. 修改 GLQueueRunner.cpp - 使用标准 glCopyImageSubData

OHOS 支持 OpenGL ES 3.2，使用标准的 `glCopyImageSubData`（不是 OES 扩展）：

```cpp
#if defined(USING_GLES2)
#if PPSSPP_PLATFORM(OHOS)
	// OHOS 支持 OpenGL ES 3.2，使用标准的 glCopyImageSubData（不是 OES 扩展）
	glCopyImageSubData(
		srcTex, target, srcLevel, srcRect.x, srcRect.y, srcZ,
		dstTex, target, dstLevel, dstPos.x, dstPos.y, dstZ,
		srcRect.w, srcRect.h, depth);
#elif !PPSSPP_PLATFORM(IOS)
	glCopyImageSubDataOES(
		srcTex, target, srcLevel, srcRect.x, srcRect.y, srcZ,
		dstTex, target, dstLevel, dstPos.x, dstPos.y, dstZ,
		srcRect.w, srcRect.h, depth);
#endif
```

### 4. 修改 GLMemory.cpp - 排除 glBufferStorageEXT

```cpp
if (gl_extensions.ARB_buffer_storage || gl_extensions.EXT_buffer_storage) {
#if !PPSSPP_PLATFORM(IOS) && !PPSSPP_PLATFORM(OHOS)
    // OHOS 不使用 glBufferStorageEXT
    if (!hasStorage_) {
        // ...
    }
#endif
    p = glMapBufferRange(target_, 0, size_, access);
}
```

### 5. 修改 thin3d_gl.cpp - 启用 framebufferCopySupported

对于 OpenGL ES 3.2，`glCopyImageSubData` 是核心函数：

```cpp
// glCopyImageSubData is core in OpenGL ES 3.2 and OpenGL 4.3+
// OHOS supports ES 3.2 with core glCopyImageSubData
caps_.framebufferCopySupported = gl_extensions.OES_copy_image || gl_extensions.NV_copy_image 
    || gl_extensions.EXT_copy_image || gl_extensions.ARB_copy_image
    || (gl_extensions.IsGLES && gl_extensions.GLES3 && gl_extensions.ver[0] >= 3 && gl_extensions.ver[1] >= 2);
```

## 技术细节

### 为什么 iOS 和 OHOS 不需要 gl3stubInit？

| 平台 | GL ES 3.0 支持方式 | gl3stubInit 需要？ |
|------|-------------------|-------------------|
| Android | 动态加载 (eglGetProcAddress) | ✅ 是 |
| iOS | 系统框架直接提供 | ❌ 否 |
| OHOS | 直接链接 GLESv3 库 | ❌ 否 |
| Linux | GLEW 或动态加载 | 取决于配置 |

### OHOS OpenGL ES 3.2 支持

鸿蒙系统支持 OpenGL ES 3.2，提供以下标准函数：
- `glCopyImageSubData` - 核心函数（不是 OES 扩展）
- `glReadBuffer`, `glTexImage3D` 等 - 所有 ES 3.0+ 核心函数

**重要：** OHOS 不支持某些 OES/EXT 扩展函数：
- ❌ `glCopyImageSubDataOES` - 使用 `glCopyImageSubData` 代替
- ❌ `glBufferStorageEXT` - 不可用
- ❌ `glBindFragDataLocationIndexedEXT` - 不可用

### 鸿蒙 OpenGL ES 头文件

鸿蒙系统提供标准的 GLES3 头文件：
- `<GLES3/gl3.h>` - GL ES 3.0 核心函数
- `<GLES3/gl31.h>` - GL ES 3.1 扩展
- `<GLES3/gl32.h>` - GL ES 3.2 扩展

这些头文件中已经声明了所有需要的函数，无需额外的函数指针声明。

## 相关文件

- `Common/GPU/OpenGL/GLCommon.h` - OpenGL 头文件包含
- `Common/GPU/OpenGL/gl3stub.c` - OpenGL ES 3.0 函数指针加载
- `Common/GPU/OpenGL/gl3stub.h` - 函数指针声明
- `Common/GPU/OpenGL/GLQueueRunner.cpp` - glCopyImageSubData 调用
- `Common/GPU/OpenGL/GLMemory.cpp` - glBufferStorageEXT 调用
- `Common/GPU/OpenGL/thin3d_gl.cpp` - framebufferCopySupported 设置
- `ohos/entry/src/main/cpp/CMakeLists.txt` - 链接 GLESv3 库
- `ppsspp_config.h` - 平台宏定义 (PPSSPP_PLATFORM_OHOS)

## 测试验证

修复后需要验证：

1. ✅ 应用能够正常启动，不再崩溃
2. ✅ OpenGL ES 3.0/3.2 功能正常工作
3. ✅ 游戏渲染正常显示
4. ✅ `gl3stubInit()` 返回 `GL_TRUE`
5. ✅ `glCopyImageSubData` 正常工作（framebuffer copy）

## 参考

- [HarmonyOS OpenGL ES 开发指南](https://developer.harmonyos.com/)
- PPSSPP iOS 实现参考
- OHOS Native API 符号列表
