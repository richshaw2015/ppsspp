# 内存初始化修复验证指南

## 快速验证步骤

### 1. 重新编译项目
```bash
cd ohos
# 清理旧的构建
rm -rf entry/.cxx

# 重新构建
hvigorw assembleHap
```

### 2. 安装并运行
```bash
# 安装到设备
hdc install entry/build/default/outputs/default/entry-default-signed.hap

# 查看日志
hdc shell hilog -c  # 清空日志
hdc shell hilog | grep PPSSPP
```

### 3. 检查日志输出

#### 成功的日志标志
```
I/PPSSPP_MemMap: HarmonyOS: Prepared anonymous memory space of size: 04000000
I/PPSSPP_MemMap: HarmonyOS 64-bit: Got base at 0x...
I/PPSSPP_MemMap: HarmonyOS 64-bit: Aligned base: 0x...
I/PPSSPP_MemMap: HarmonyOS: Created memory view at 0x... (requested: 0x...), size: 00010000
I/PPSSPP_MemMap: Memory system initialized. Base at 0x... (RAM at @ 0x..., uncached @ 0x...)
```

#### 失败的日志标志
```
E/PPSSPP_MemMap: HarmonyOS mmap failed: base=0x..., size=..., errno=... (...)
E/PPSSPP_System: Memory init failed
```

### 4. 测试游戏加载

1. 打开 PPSSPP 应用
2. 浏览并选择一个 PSP 游戏（ISO/CSO 文件）
3. 点击游戏图标启动

**预期结果**：
- ✅ 游戏成功加载，进入游戏画面
- ✅ 没有 "memory init failed" 错误

**如果仍然失败**：
- ❌ 查看下一节的故障排除

## 故障排除

### 问题 1: mmap 返回 ENOMEM (内存不足)

**症状**：
```
E/PPSSPP_MemMap: HarmonyOS mmap failed: errno=12 (Cannot allocate memory)
```

**可能原因**：
- 设备内存不足
- 内存限制过低

**解决方案**：
1. 关闭其他应用释放内存
2. 检查应用的内存限制配置
3. 尝试减少内存分配大小（修改 `g_MemorySize`）

### 问题 2: mmap 返回 EINVAL (无效参数)

**症状**：
```
E/PPSSPP_MemMap: HarmonyOS mmap failed: errno=22 (Invalid argument)
```

**可能原因**：
- 地址对齐问题
- 大小参数错误
- 标志组合不支持

**解决方案**：
1. 检查 `Find4GBBase()` 返回的地址是否有效
2. 验证内存大小是否合理
3. 尝试不使用 `MAP_FIXED` 标志

### 问题 3: 编译错误

**症状**：
```
error: 'MemArenaOhos.cpp' not found
```

**解决方案**：
1. 确认文件已创建：`Common/MemArenaOhos.cpp`
2. 确认 CMake 配置已更新：
   - `ohos/entry/src/main/cpp/common_sources.cmake`
   - `CMakeLists.txt`
3. 清理并重新构建

### 问题 4: 多个 MemArena 实现冲突

**症状**：
```
error: multiple definition of 'MemArena::GrabMemSpace'
```

**解决方案**：
确保只有一个 MemArena 实现被编译：
- ✅ `MemArenaOhos.cpp` (OHOS)
- ❌ `MemArenaPosix.cpp` (不应该被编译)
- ❌ `MemArenaAndroid.cpp` (不应该被编译)

检查 `common_sources.cmake` 中只包含 `MemArenaOhos.cpp`。

## 调试技巧

### 1. 增加日志级别
在 `ohos_app.cpp` 的 `Initialize()` 函数中：
```cpp
// 启用详细日志
g_logManager.SetAllLogLevels(LogLevel::LDEBUG);
```

### 2. 检查内存映射
在设备上查看进程的内存映射：
```bash
# 获取 PPSSPP 进程 PID
hdc shell ps -ef | grep ppsspp

# 查看内存映射
hdc shell cat /proc/<PID>/maps
```

### 3. 测试简化版本
临时修改 `Core/MemMap.cpp` 减少内存分配：
```cpp
// 临时使用最小内存配置
g_MemorySize = RAM_NORMAL_SIZE;  // 24MB instead of 64MB
```

### 4. 单步调试
使用 DevEco Studio 的调试器：
1. 在 `Memory::Init()` 设置断点
2. 在 `MemArena::GrabMemSpace()` 设置断点
3. 在 `MemArena::CreateView()` 设置断点
4. 逐步执行，检查返回值

## 性能验证

### 内存使用
检查应用的内存占用：
```bash
hdc shell dumpsys meminfo <package_name>
```

**预期**：
- PSS Total: ~100-200 MB（取决于游戏）
- Native Heap: ~50-100 MB

### 启动时间
测量从点击游戏到进入游戏画面的时间：
- **目标**: < 5 秒
- **可接受**: < 10 秒
- **需要优化**: > 10 秒

## 回归测试

确保修复没有破坏其他功能：

### 基本功能
- [ ] 应用启动
- [ ] 游戏列表显示
- [ ] 游戏加载
- [ ] 游戏运行
- [ ] 保存/加载状态
- [ ] 设置修改

### 内存相关
- [ ] 长时间运行无内存泄漏
- [ ] 切换游戏正常
- [ ] 暂停/恢复正常
- [ ] 应用后台/前台切换正常

## 成功标准

修复被认为成功，当：

1. ✅ 游戏可以正常加载（无 "memory init failed" 错误）
2. ✅ 内存使用合理（< 200MB）
3. ✅ 无内存泄漏
4. ✅ 性能正常（FPS 稳定）
5. ✅ 所有回归测试通过

## 下一步

如果内存初始化成功，但游戏仍然无法运行，检查：
1. 图形渲染（OpenGL/Vulkan）
2. 音频输出
3. 输入处理
4. 文件系统访问
5. 游戏兼容性

参考其他文档：
- `OPENGL_IMPLEMENTATION.md` - OpenGL 渲染
- `VULKAN_IMPLEMENTATION.md` - Vulkan 渲染
- `INPUT_SYSTEM_IMPLEMENTATION.md` - 输入系统
- `OHOS_ADAPTATION_STATUS.md` - 整体适配状态
