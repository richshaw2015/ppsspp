# PPSSPP 鸿蒙平台适配实施计划

## 总体目标

将 PPSSPP（PSP 模拟器）完整移植到鸿蒙平台，实现与 Android 版本相当的功能和性能。

## 阶段划分

### 阶段 0: 基础框架验证（当前阶段）✅

**目标**: 确保基础框架可以编译和运行

**任务清单**:
- [x] 创建项目结构
- [x] 实现 NAPI 接口框架
- [x] 实现平台适配层框架
- [x] 配置 CMake 构建系统
- [ ] 验证编译通过
- [ ] 验证基础 NAPI 调用

**验证标准**:
```bash
# 编译成功
cd ohos
hvigorw assembleHap

# 安装运行
hvigorw installHapDebug

# 日志验证
hdc shell hilog | grep PPSSPP
```

**预计时间**: 1-2 天

---

### 阶段 1: 集成 PPSSPP 依赖库（2-3 周）

**目标**: 成功编译 PPSSPP 的依赖库

#### 1.1 集成 ext 第三方库

**任务**:
- [ ] 修改 ext/CMakeLists.txt 支持鸿蒙
- [ ] 编译 snappy（压缩库）
- [ ] 编译 glslang（着色器编译器）
- [ ] 编译 libzip（ZIP 文件支持）
- [ ] 编译 libpng（PNG 图像）
- [ ] 编译 zstd（压缩）

**关键文件**:
```cmake
# ohos/entry/src/main/cpp/CMakeLists.txt
add_subdirectory(${PPSSPP_ROOT}/ext ${CMAKE_BINARY_DIR}/ext)
```

**验证**: 所有 ext 库编译成功，无链接错误

#### 1.2 集成 Common 库

**任务**:
- [ ] 适配 Common/System/System.h 接口
- [ ] 实现 Common/System/NativeApp.h 接口
- [ ] 适配文件系统接口
- [ ] 适配网络接口（可选）

**关键实现**:
```cpp
// 实现 NativeApp.h 中的函数
void NativeInit(int argc, const char *argv[], ...);
void NativeShutdown();
void NativeUpdate();
void NativeRender(GraphicsContext *graphicsContext);
```

**验证**: Common 库编译成功并链接

#### 1.3 集成 Core 库

**任务**:
- [ ] 编译 Core 模拟器核心
- [ ] 编译 GPU 模块
- [ ] 编译 HLE 模块
- [ ] 编译 MIPS JIT（如果支持）

**验证**: Core 库编译成功，符号正确导出

**预计时间**: 2-3 周

---

### 阶段 2: 图形渲染实现（2-3 周）

**目标**: 实现完整的 OpenGL ES 渲染管线

#### 2.1 完善 OpenGL ES 上下文

**任务**:
- [ ] 实现 XComponent 回调处理
- [ ] 实现表面创建/销毁
- [ ] 实现表面尺寸变化处理
- [ ] 实现 VSync 同步

**关键代码**:
```cpp
// ohos_gl_context.cpp
void OhosGLContext::OnSurfaceCreated(OH_NativeXComponent* component) {
    // 获取 native window
    // 创建 EGL surface
    // 绑定上下文
}

void OhosGLContext::OnSurfaceChanged(int width, int height) {
    // 更新视口
    // 通知 PPSSPP Core
}
```

#### 2.2 实现 GraphicsContext 接口

**任务**:
- [ ] 继承 Common/GraphicsContext.h
- [ ] 实现 Thin3D 接口
- [ ] 实现帧缓冲管理
- [ ] 实现纹理管理

**参考**: `android/jni/AndroidGraphicsContext.h`

#### 2.3 渲染循环 ✅

**任务**:
- [x] 实现主渲染循环
- [x] 实现帧率控制（60 FPS 目标）
- [x] 实现帧统计（FPS + 帧时间）

**完成时间**: 2024-12-08

**实现细节**:
- 独立渲染线程
- 智能帧率控制（目标 60 FPS）
- 实时帧统计（每秒更新）
- 性能监控日志（每 5 秒）
- 线程安全设计
- 参考 Android 实现

**文档**: [RENDER_LOOP.md](../RENDER_LOOP_COMPLETE.md)

**关键代码**:
```cpp
void OhosApp::RunFrame() {
    if (!IsRunning()) return;
    
    // 1. 处理输入
    // 2. 更新模拟器状态
    Core_UpdateState();
    
    // 3. 渲染
    if (graphicsContext) {
        NativeRender(graphicsContext);
        graphicsContext->SwapBuffers();
    }
    
    // 4. 输出音频
}
```

**验证**: 能够显示 PPSSPP 的 UI 界面

**预计时间**: 2-3 周

---

### 阶段 3: 音频系统实现（1-2 周）

**目标**: 实现低延迟音频输出

#### 3.1 AudioRenderer 集成

**任务**:
- [ ] 创建 AudioRenderer 实例
- [ ] 配置音频参数（44100Hz, 立体声, 16-bit）
- [ ] 实现音频回调
- [ ] 实现缓冲区管理

**关键代码**:
```cpp
// ohos_audio.cpp
#include <multimedia/player_framework/native_avcodec_audiorenderer.h>

bool OhosAudio::Initialize(int sampleRate, int bufferSize) {
    // 创建 AudioRenderer
    OH_AudioRenderer_Create(&renderer, ...);
    
    // 设置回调
    OH_AudioRenderer_SetCallback(renderer, AudioCallback, this);
    
    // 启动
    OH_AudioRenderer_Start(renderer);
}

void AudioCallback(OH_AudioRenderer* renderer, void* userData, 
                   OH_AudioBuffer* buffer) {
    // 从 PPSSPP Core 获取音频数据
    // 填充到 buffer
}
```

#### 3.2 音频同步

**任务**:
- [ ] 实现音频/视频同步
- [ ] 处理音频延迟
- [ ] 实现动态缓冲区调整

**验证**: 游戏音频流畅播放，无明显延迟或卡顿

**预计时间**: 1-2 周

---

### 阶段 4: 输入系统实现（1 周）

**目标**: 完整的输入支持

#### 4.1 触摸输入

**任务**:
- [ ] 映射触摸事件到 PPSSPP 输入
- [ ] 实现虚拟按键检测
- [ ] 实现多点触控
- [ ] 实现手势识别

**关键代码**:
```cpp
void OhosInput::HandleTouchEvent(float x, float y, TouchAction action, int id) {
    // 转换坐标
    float scaledX = x * displayScale;
    float scaledY = y * displayScale;
    
    // 检查虚拟按键
    int button = CheckVirtualButton(scaledX, scaledY);
    if (button >= 0) {
        // 发送按键事件到 Core
        NativeKey(KeyInput{...});
    } else {
        // 发送触摸事件到 Core
        NativeTouch(TouchInput{...});
    }
}
```

#### 4.2 物理按键和手柄

**任务**:
- [ ] 映射物理按键
- [ ] 支持外接手柄
- [ ] 实现按键配置

**验证**: 触摸、按键、手柄输入正常工作

**预计时间**: 1 周

---

### 阶段 5: 文件系统和游戏加载（1 周）

**目标**: 能够加载和运行 PSP 游戏

#### 5.1 文件访问

**任务**:
- [ ] 实现文件选择器
- [ ] 处理文件权限
- [ ] 支持 ISO/CSO/PBP 格式
- [ ] 实现最近游戏列表

**关键代码**:
```typescript
// GameView.ets
async selectGame() {
  try {
    let result = await picker.select({
      type: 'file',
      maxSelectNumber: 1
    });
    
    if (result && result.length > 0) {
      PPSSPPNative.loadGame(result[0]);
    }
  } catch (error) {
    console.error('Select game failed:', error);
  }
}
```

#### 5.2 保存数据管理

**任务**:
- [ ] 实现存档保存/加载
- [ ] 实现即时存档
- [ ] 实现配置保存

**验证**: 能够加载游戏并正常运行

**预计时间**: 1 周

---

### 阶段 6: UI 界面开发（2-3 周）

**目标**: 完整的用户界面

#### 6.1 主界面

**任务**:
- [ ] 游戏列表视图
- [ ] 游戏封面显示
- [ ] 搜索和过滤
- [ ] 最近游戏

#### 6.2 游戏内 UI

**任务**:
- [ ] 虚拟按键布局
- [ ] 暂停菜单
- [ ] 快速设置
- [ ] FPS 显示

#### 6.3 设置界面

**任务**:
- [ ] 图形设置
- [ ] 音频设置
- [ ] 控制设置
- [ ] 系统设置

**预计时间**: 2-3 周

---

### 阶段 7: 优化和测试（2-3 周）

**目标**: 性能优化和稳定性提升

#### 7.1 性能优化

**任务**:
- [ ] CPU 优化（NEON 指令）
- [ ] GPU 优化（减少状态切换）
- [ ] 内存优化（减少分配）
- [ ] 电池优化

**工具**:
- DevEco Studio Profiler
- HiTrace
- HiPerf

#### 7.2 兼容性测试

**任务**:
- [ ] 测试主流游戏（50+ 款）
- [ ] 测试不同设备
- [ ] 测试不同分辨率
- [ ] 压力测试

#### 7.3 稳定性测试

**任务**:
- [ ] 长时间运行测试
- [ ] 内存泄漏检测
- [ ] 崩溃日志分析
- [ ] 边界条件测试

**预计时间**: 2-3 周

---

## 总时间估算

- 阶段 0: 1-2 天
- 阶段 1: 2-3 周
- 阶段 2: 2-3 周
- 阶段 3: 1-2 周
- 阶段 4: 1 周
- 阶段 5: 1 周
- 阶段 6: 2-3 周
- 阶段 7: 2-3 周

**总计**: 10-16 周（约 2.5-4 个月）

## 里程碑

1. **M1 - 基础框架** (Week 2): 编译通过，基础 NAPI 调用成功
2. **M2 - Core 集成** (Week 5): PPSSPP Core 成功集成
3. **M3 - 首次渲染** (Week 8): 能够显示 PPSSPP UI
4. **M4 - 首个游戏** (Week 10): 能够加载并运行第一个游戏
5. **M5 - 功能完整** (Week 13): 所有基础功能实现
6. **M6 - 发布就绪** (Week 16): 优化完成，可以发布

## 风险和挑战

### 高风险

1. **PPSSPP Core 集成复杂度**: 可能需要大量适配工作
   - 缓解: 参考 Android 实现，逐步集成
   
2. **性能问题**: 鸿蒙平台性能可能不如预期
   - 缓解: 早期进行性能测试，及时优化

3. **图形 API 差异**: EGL/OpenGL ES 实现可能有差异
   - 缓解: 使用标准 API，避免平台特定功能

### 中风险

1. **音频延迟**: 音频系统可能存在延迟问题
   - 缓解: 使用低延迟模式，调整缓冲区

2. **文件系统限制**: 鸿蒙文件访问可能有限制
   - 缓解: 使用标准 API，申请必要权限

3. **调试困难**: 鸿蒙平台调试工具可能不够完善
   - 缓解: 使用日志，远程调试

## 资源需求

### 人力

- 核心开发: 1-2 人
- UI 开发: 1 人
- 测试: 1 人

### 设备

- 鸿蒙手机（至少 2 台不同型号）
- 鸿蒙平板（可选）
- 开发机（Mac/Windows/Linux）

### 工具

- DevEco Studio
- HarmonyOS SDK
- Git
- 性能分析工具

## 下一步行动

### 立即执行（本周）

1. ✅ 验证基础框架编译
2. [ ] 开始集成 ext 库
3. [ ] 编写单元测试框架
4. [ ] 搭建 CI/CD 流程

### 短期目标（2 周内）

1. [ ] 完成 ext 库集成
2. [ ] 开始 Common 库适配
3. [ ] 实现基础的 GraphicsContext

### 中期目标（1 个月内）

1. [ ] 完成 Core 库集成
2. [ ] 实现完整的渲染管线
3. [ ] 实现音频输出

## 成功标准

### 最小可行产品 (MVP)

- [ ] 能够加载 ISO 游戏
- [ ] 能够运行至少 10 款主流游戏
- [ ] 帧率达到 30 FPS 以上
- [ ] 音频正常播放
- [ ] 触摸控制正常

### 完整版本

- [ ] 支持 50+ 款游戏
- [ ] 帧率达到 60 FPS
- [ ] 完整的 UI 界面
- [ ] 保存/读取功能
- [ ] 虚拟按键自定义
- [ ] 手柄支持
- [ ] 性能优化完成

## 参考资料

- [PPSSPP 开发文档](https://github.com/hrydgard/ppsspp/wiki)
- [HarmonyOS 开发指南](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5)
- [Android 移植参考](android/jni/)
- [iOS 移植参考](ios/)
