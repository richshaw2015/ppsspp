# OHOS 平台图片选择器实现

## 概述

实现了 OHOS 平台的图片选择功能，用于 PPSSPP 的自定义壁纸设置等需要选择图片的场景。

## 架构

```
PPSSPP UI (C++ 渲染线程)
    ↓
System_BrowseForImage()
    ↓
g_requestManager.MakeSystemRequest(BROWSE_FOR_IMAGE, requestId, ...)
    ↓
System_MakeRequest() [ohos_system_properties.cpp]
    ↓
NapiPPSSPP::BrowseForImage(requestId)
    ↓
napi_threadsafe_function (线程安全队列)
    ↓
BrowseImageCallJs() [主线程执行]
    ↓
ArkTS 回调 (Index.ets)
    ↓
PhotoViewPicker.select()
    ↓
copyImageToSandbox() [复制到应用沙箱]
    ↓
ppsspp.onImageSelected(requestId, success, path)
    ↓
NapiPPSSPP::OnImageSelected()
    ↓
g_requestManager.PostSystemSuccess/PostSystemFailure()
```

## 关键文件

### C++ 层

1. **ohos/entry/src/main/cpp/ohos_system_properties.cpp**
   - `System_MakeRequest()` - 处理 `BROWSE_FOR_IMAGE` 请求

2. **ohos/entry/src/main/cpp/napi/napi_ppsspp.cpp**
   - `SetBrowseImageCallback()` - 注册 ArkTS 回调
   - `BrowseForImage()` - 从渲染线程调用，通过线程安全函数发送到主线程
   - `OnImageSelected()` - 接收 ArkTS 层的选择结果

3. **ohos/entry/src/main/cpp/napi/napi_ppsspp.h**
   - 函数声明

4. **ohos/entry/src/main/cpp/napi/napi_init.cpp**
   - 注册 NAPI 函数

### ArkTS 层

1. **ohos/entry/src/main/ets/pages/Index.ets**
   - `setupBrowseImageCallback()` - 设置图片选择回调
   - `copyImageToSandbox()` - 将选择的图片复制到应用沙箱

2. **ohos/entry/src/main/ets/types/libppsspp.d.ts**
   - TypeScript 类型声明

## 线程安全

与 OpenUrl 实现类似，使用 `napi_threadsafe_function` 确保从渲染线程安全调用 ArkTS 代码：

1. 渲染线程调用 `BrowseForImage(requestId)`
2. 通过 `napi_call_threadsafe_function` 将请求放入队列
3. 主线程执行 `BrowseImageCallJs()` 回调
4. ArkTS 层打开图片选择器
5. 选择完成后调用 `onImageSelected()` 通知 C++ 层

## 图片处理

由于 PPSSPP 需要直接访问文件路径，而 PhotoPicker 返回的是 URI，因此需要：

1. 使用 `fileIo.open()` 打开 URI
2. 复制文件内容到应用沙箱目录
3. 返回沙箱中的文件路径给 PPSSPP

## 使用的 OHOS API

- `@kit.MediaLibraryKit` - `photoAccessHelper.PhotoViewPicker`
- `@kit.CoreFileKit` - `fileIo`

