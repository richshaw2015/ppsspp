# OHOS 平台打开外部链接实现

## 功能说明

实现了 OHOS 平台点击链接跳转到外部浏览器打开的功能，例如首页的"如何获得游戏？"链接。

## 实现架构

```
PPSSPP UI 层 (C++) - 渲染线程
    ↓
System_LaunchUrl()  [ohos_system_stubs.cpp]
    ↓
NapiPPSSPP::OpenUrl()  [napi_ppsspp.cpp]
    ↓
napi_threadsafe_function  (线程安全队列)
    ↓
OpenUrlCallJs()  [主线程执行]
    ↓
ArkTS 回调函数  [Index.ets]
    ↓
context.startAbility(want)  [鸿蒙系统 API]
    ↓
外部浏览器
```

## 关键技术点：线程安全

鸿蒙的 JS 引擎（ArkTS）只能在主线程运行，但 PPSSPP 的 UI 事件（如点击链接）
是在渲染线程触发的。直接从渲染线程调用 NAPI 回调会导致崩溃：

```
ecma_vm cannot run in multi-thread!
```

解决方案是使用 `napi_threadsafe_function`：
1. 在主线程注册回调时创建线程安全函数
2. 从任意线程调用 `napi_call_threadsafe_function` 发送请求
3. 请求会被放入队列，在主线程的事件循环中执行

## 修改的文件

### 1. C++ 层

#### ohos/entry/src/main/cpp/napi/napi_ppsspp.h
- 添加 `SetOpenUrlCallback` 函数声明
- 添加 `OpenUrl` 函数声明

#### ohos/entry/src/main/cpp/napi/napi_ppsspp.cpp
- 实现 `SetOpenUrlCallback` - 保存 ArkTS 回调函数引用
- 实现 `OpenUrl` - 调用 ArkTS 回调函数打开 URL

#### ohos/entry/src/main/cpp/ohos_system_stubs.cpp
- 修改 `System_LaunchUrl` - 调用 `NapiPPSSPP::OpenUrl`

#### ohos/entry/src/main/cpp/napi/napi_init.cpp
- 注册 `setOpenUrlCallback` 函数到 NAPI 导出

### 2. ArkTS 层

#### ohos/entry/src/main/ets/types/libppsspp.d.ts
- 添加 `setOpenUrlCallback` 类型声明

#### ohos/entry/src/main/ets/pages/Index.ets
- 导入 `Want` 类型
- 添加 `setupOpenUrlCallback` 方法
- 在初始化时调用 `setupOpenUrlCallback`

## 使用方式

PPSSPP 中所有调用 `System_LaunchUrl` 的地方都会自动使用此功能：

```cpp
// 例如：打开"如何获得游戏？"链接
System_LaunchUrl(LaunchUrlType::BROWSER_URL, "https://www.ppsspp.org/getgames");
```

## 鸿蒙 API 说明

使用 `Want` 和 `startAbility` 打开外部浏览器：

```typescript
const want: Want = {
  action: 'ohos.want.action.viewData',
  uri: url
};
this.context.startAbility(want);
```

## 支持的 URL 类型

- `LaunchUrlType::BROWSER_URL` - 在浏览器中打开网页
- `LaunchUrlType::MARKET_URL` - 打开应用市场（使用相同方式）
- `LaunchUrlType::EMAIL_ADDRESS` - 打开邮件客户端（使用相同方式）

## 测试方法

1. 启动 PPSSPP
2. 在主界面点击"如何获得游戏？"链接
3. 应该会跳转到外部浏览器打开对应网页
