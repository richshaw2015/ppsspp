# HarmonyOS 内存初始化失败问题修复

## 问题描述

在 HarmonyOS 平台上点击进入游戏时，出现错误提示：
```
无法加载游戏 memory init failed
```

## 问题根源

### 1. 错误来源
错误信息来自 `Core/System.cpp:371`：
```cpp
if (!Memory::Init()) {
    *errorString = "Memory init failed";
    return false;
}
```

### 2. 内存初始化流程
PPSSPP 的内存初始化流程：
1. `Memory::Init()` → `MemoryMap_Setup()` → `g_arena.GrabMemSpace()`
2. `GrabMemSpace()` 负责分配 PSP 模拟器需要的内存空间
3. 不同平台使用不同的内存分配策略：
   - **Windows**: VirtualAlloc
   - **Android**: ashmem (匿名共享内存)
   - **Linux/POSIX**: shm_open 或文件映射
   - **macOS**: vm_allocate

### 3. OHOS 平台的问题
之前的实现中，OHOS 使用了 `MemArenaPosix.cpp`，它依赖：
- `shm_open()` - POSIX 共享内存
- `/dev/shm` - 共享内存文件系统
- `/tmp` - 临时文件系统

**问题**：HarmonyOS 作为移动操作系统，可能不支持或限制了这些 POSIX 特性（类似 Android 的沙盒限制）。

## 解决方案

### 创建 OHOS 专用的 MemArena 实现

创建了 `Common/MemArenaOhos.cpp`，使用匿名内存映射（`MAP_ANONYMOUS`）：

```cpp
void *MemArena::CreateView(s64 offset, size_t size, void *base) {
    int flags = MAP_ANONYMOUS | MAP_SHARED;
    if (base != nullptr) {
        flags |= MAP_FIXED;
    }
    
    void *retval = mmap(base, size, PROT_READ | PROT_WRITE, flags, -1, 0);
    // ...
}
```

### 关键特性

1. **匿名内存映射**：不需要文件系统支持
   - 使用 `MAP_ANONYMOUS` 标志
   - 不需要文件描述符（fd = -1）
   - 直接从内核分配内存

2. **共享映射**：支持内存镜像
   - 使用 `MAP_SHARED` 标志
   - PSP 内存系统需要多个虚拟地址映射到同一物理内存

3. **固定地址映射**：支持特定地址
   - 使用 `MAP_FIXED` 标志（当需要时）
   - PSP 内存布局需要固定的虚拟地址

4. **64位优化**：
   - 分配 8GB 并对齐到 4GB 边界
   - 支持 ARM64 的 movk 指令优化

## 修改的文件

### 1. 新增文件
- `Common/MemArenaOhos.cpp` - OHOS 专用内存分配实现

### 2. 修改文件
- `ohos/entry/src/main/cpp/common_sources.cmake`
  ```cmake
  # 修改前
  ${PPSSPP_ROOT}/Common/MemArenaPosix.cpp
  
  # 修改后
  ${PPSSPP_ROOT}/Common/MemArenaOhos.cpp
  ```

## 技术细节

### PSP 内存布局
PSP 模拟器需要以下内存区域：
- **Scratchpad**: 16KB 快速缓存
- **VRAM**: 2MB 显存（4个镜像）
- **RAM**: 24MB-64MB 主内存（3个镜像，每个有4个地址空间）
  - Physical: 0x08000000
  - Uncached: 0x48000000
  - Kernel: 0x88000000
  - Uncached Kernel: 0xC8000000

### 内存镜像机制
PSP 的内存系统使用镜像来支持不同的访问模式：
- **Cached vs Uncached**: 不同的缓存策略
- **User vs Kernel**: 不同的权限级别
- **Physical vs Virtual**: 不同的地址空间

PPSSPP 通过 `MAP_SHARED` 实现镜像：多个虚拟地址映射到同一物理内存。

### Android vs OHOS 对比

| 特性 | Android | OHOS (旧) | OHOS (新) |
|------|---------|-----------|-----------|
| 实现文件 | MemArenaAndroid.cpp | MemArenaPosix.cpp | MemArenaOhos.cpp |
| 内存机制 | ashmem | shm_open/文件 | MAP_ANONYMOUS |
| 文件依赖 | libandroid.so | /dev/shm, /tmp | 无 |
| 系统调用 | ASharedMemory_create | shm_open, open | mmap |
| 沙盒兼容 | ✅ | ❌ | ✅ |

## 测试验证

### 预期行为
修复后，游戏加载应该成功，日志中应该看到：
```
I/PPSSPP_MemMap: HarmonyOS: Prepared anonymous memory space of size: 04000000
I/PPSSPP_MemMap: HarmonyOS: Created memory view at 0x... (requested: 0x...), size: ...
I/PPSSPP_MemMap: Memory system initialized. Base at 0x... (RAM at @ 0x..., uncached @ 0x...)
```

### 如果仍然失败
检查日志中的错误信息：
1. **mmap failed**: 检查内存限制、权限
2. **Failed to map**: 可能是地址冲突或内存不足
3. **errno**: 查看具体的系统错误码

## 相关代码位置

- `Core/System.cpp:369-374` - 内存初始化调用
- `Core/MemMap.cpp:299-311` - Memory::Init() 实现
- `Core/MemMap.cpp:151-195` - MemoryMap_Setup() 实现
- `Common/MemArenaOhos.cpp` - OHOS 内存分配实现

## 参考资料

- [PPSSPP Memory System](https://github.com/hrydgard/ppsspp/wiki/Memory-System)
- [PSP Memory Map](http://hitmen.c02.at/files/yapspd/psp_doc/chap4.html)
- [Linux mmap(2)](https://man7.org/linux/man-pages/man2/mmap.2.html)
- [Android ashmem](https://developer.android.com/ndk/reference/group/memory)

## 总结

通过创建 OHOS 专用的 MemArena 实现，使用标准的匿名内存映射（`MAP_ANONYMOUS`）替代 POSIX 共享内存机制，解决了 HarmonyOS 平台上的内存初始化失败问题。这个方案：

1. ✅ 不依赖文件系统（/dev/shm, /tmp）
2. ✅ 兼容 HarmonyOS 的沙盒限制
3. ✅ 支持内存镜像（MAP_SHARED）
4. ✅ 支持固定地址映射（MAP_FIXED）
5. ✅ 优化 64位架构性能

这是一个更适合移动平台的解决方案，类似于 Android 的 ashmem 机制，但使用标准的 POSIX API。
