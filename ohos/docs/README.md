# PPSSPP 鸿蒙平台适配

## 项目结构

```
ohos/
├── entry/                          # 应用入口模块
│   ├── src/main/
│   │   ├── ets/                   # ArkTS 代码
│   │   │   ├── entryability/      # 应用能力
│   │   │   ├── pages/             # UI 页面
│   │   │   │   └── GameView.ets   # 游戏渲染视图
│   │   │   └── napi/              # NAPI 接口封装
│   │   │       └── PPSSPPNative.ets
│   │   ├── cpp/                   # Native C++ 代码
│   │   │   ├── napi/              # NAPI 绑定
│   │   │   │   ├── napi_init.cpp
│   │   │   │   └── napi_ppsspp.cpp
│   │   │   ├── ohos_app.cpp       # 应用层实现
│   │   │   ├── ohos_system.cpp    # 系统接口
│   │   │   ├── ohos_audio.cpp     # 音频后端
│   │   │   ├── ohos_gl_context.cpp # OpenGL 上下文
│   │   │   ├── ohos_input.cpp     # 输入处理
│   │   │   └── CMakeLists.txt     # 构建配置
│   │   └── resources/             # 资源文件
│   └── build-profile.json5        # 模块构建配置
└── build-profile.json5            # 项目构建配置
```

## 构建步骤

### 1. 环境准备

- DevEco Studio 5.0+
- HarmonyOS SDK API 12+
- NDK 工具链

### 2. 编译项目

```bash
# 在 DevEco Studio 中打开项目
# 选择 File -> Open -> 选择 ohos 目录

# 或使用命令行
cd ohos
hvigorw assembleHap
```

### 3. 运行测试

```bash
# 连接鸿蒙设备或启动模拟器
hdc list targets

# 安装应用
hvigorw installHapDebug
```

## 当前实现状态

### ✅ 已完成

- [x] 基础项目结构
- [x] NAPI 接口框架
- [x] 系统接口封装
- [x] 输入系统框架
- [x] 音频系统框架
- [x] OpenGL ES 上下文
- [x] ArkTS UI 框架

### 🚧 进行中

- [ ] PPSSPP Core 集成
- [ ] 图形渲染实现
- [ ] 音频输出实现
- [ ] 输入事件映射
- [ ] 文件系统完善

### 📋 待实现

- [ ] 游戏加载功能
- [ ] 保存/读取状态
- [ ] 虚拟按键
- [ ] 设置界面
- [ ] 游戏列表
- [ ] 性能优化
- [ ] Vulkan 支持（可选）

## 开发指南

### 添加新的 NAPI 接口

1. 在 `napi/napi_ppsspp.h` 中声明函数
2. 在 `napi/napi_ppsspp.cpp` 中实现函数
3. 在 `napi/napi_init.cpp` 中注册函数
4. 在 `ets/napi/PPSSPPNative.ets` 中添加 TypeScript 封装

### 调试技巧

```cpp
// 使用 HiLog 输出日志
#include <hilog/log.h>
OH_LOG_INFO(LOG_APP, "Message: %{public}s", message.c_str());
OH_LOG_ERROR(LOG_APP, "Error code: %{public}d", errorCode);
```

```typescript
// ArkTS 中查看日志
console.info('PPSSPP', 'Message');
console.error('PPSSPP', 'Error');
```

### 性能分析

- 使用 DevEco Studio 的 Profiler 工具
- 监控 CPU、内存、GPU 使用情况
- 使用 HiTrace 进行性能追踪

## 参考资料

- [HarmonyOS 开发文档](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/application-dev-guide-V5)
- [NAPI 开发指南](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/napi-guidelines-V5)
- [OpenGL ES 开发](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/opengl-overview-V5)
- [PPSSPP 项目](https://github.com/hrydgard/ppsspp)

## 常见问题

### Q: 编译失败，找不到头文件

A: 检查 CMakeLists.txt 中的 include_directories 路径是否正确

### Q: 运行时崩溃

A: 检查 HiLog 日志，确认 NAPI 接口调用是否正确

### Q: 图形渲染黑屏

A: 确认 EGL 初始化成功，检查 XComponent 配置

## 贡献指南

欢迎提交 Issue 和 Pull Request！

## 许可证

遵循 PPSSPP 项目的 GPL 2.0+ 许可证
