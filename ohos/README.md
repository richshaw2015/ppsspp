# PPSSPP 鸿蒙平台适配

本项目是 [PPSSPP](https://github.com/hrydgard/ppsspp) 的 OpenHarmony/HarmonyOS 平台适配版本。

## 许可证

本项目遵循 **GNU General Public License v2.0 (GPL-2.0)** 许可证开源。

- 原项目：[PPSSPP](https://github.com/hrydgard/ppsspp)
- 原作者：Henrik Rydgård 及众多贡献者
- ohos 适配：richshaw

根据 GPL 2.0 许可证，您可以自由地：
- 使用、复制、分发本软件
- 修改源代码并分发修改后的版本

但您必须：
- 保留原始版权声明和许可证
- 以相同的 GPL 2.0 许可证发布衍生作品
- 提供源代码或获取源代码的方式

完整许可证文本请参阅项目根目录的 [LICENSE.TXT](../LICENSE.TXT)。

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

- DevEco Studio 6.0+
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

### 3. 发布构建

```bash
hvigorw -p product=release assembleApp && open build/outputs/release/
```

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

## 参考资料

- [HarmonyOS 开发文档](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/application-dev-guide-V5)
- [NAPI 开发指南](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/napi-guidelines-V5)
- [OpenGL ES 开发](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/opengl-overview-V5)
- [PPSSPP 项目](https://github.com/hrydgard/ppsspp)

## 贡献指南

欢迎提交 Issue 和 Pull Request！

在提交代码时，请确保：
- 新增的源文件包含 GPL 2.0 版权头
- 遵循项目现有的代码风格
- 提供必要的文档说明


## 许可证详情

### 源代码许可

所有 `ohos/entry/src/main/cpp/` 目录下的源代码文件均包含以下 GPL 2.0 版权头：

```cpp
// PPSSPP - OpenHarmony Platform Adaptation
// Copyright (c) 2024-2025 richshaw
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License 2.0 for more details.
//
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/
```

### 第三方依赖

- HarmonyOS SDK API：系统库，不受 GPL 约束
- PPSSPP Core：GPL 2.0+
- 其他依赖请参阅原项目 [LICENSE.TXT](../LICENSE.TXT)
