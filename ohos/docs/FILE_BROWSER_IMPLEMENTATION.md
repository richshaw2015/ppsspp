# OHOS 文件/文件夹浏览器实现

## 概述

实现了 PPSSPP 的文件和文件夹选择功能，用于选择游戏文件、存档、游戏目录等。

## 实现的功能

| 功能 | SystemRequestType | 用途 |
|------|------------------|------|
| 文件选择 | `BROWSE_FOR_FILE` | 选择游戏 ISO、存档、音效等 |
| 文件夹选择 | `BROWSE_FOR_FOLDER` | 选择游戏目录 |
| 图片选择 | `BROWSE_FOR_IMAGE` | 选择自定义壁纸 |

## 架构设计

```
PPSSPP Core (渲染线程)
    ↓ System_MakeRequest()
ohos_system_properties.cpp
    ↓ NapiPPSSPP::BrowseForFile/Folder()
napi_ppsspp.cpp (线程安全函数)
    ↓ napi_call_threadsafe_function()
ArkTS 主线程 (Index.ets)
    ↓ DocumentViewPicker
用户选择文件/文件夹
    ↓ onFileSelected/onFolderSelected
napi_ppsspp.cpp
    ↓ g_requestManager.PostSystemSuccess/Failure()
PPSSPP Core
```

## 文件类型映射

```typescript
switch (fileType) {
  case 0: // BOOTABLE - 游戏文件
    suffixFilters = ['.iso', '.cso', '.pbp', '.elf', '.prx', '.bin'];
  case 1: // IMAGE
    suffixFilters = ['.png', '.jpg', '.jpeg', '.bmp'];
  case 2: // INI
    suffixFilters = ['.ini'];
  case 3: // DB
    suffixFilters = ['.db'];
  case 4: // SOUND_EFFECT
    suffixFilters = ['.wav', '.mp3', '.ogg'];
  case 5: // ZIP
    suffixFilters = ['.zip'];
  case 6: // SYMBOL_MAP
  case 7: // SYMBOL_MAP_NOCASH
    suffixFilters = ['.map', '.sym'];
  case 8: // ATRAC3
    suffixFilters = ['.at3', '.aa3'];
  default: // ANY
    suffixFilters = ['*'];
}
```

## 关键实现细节

### 1. 线程安全

由于 `System_MakeRequest` 从渲染线程调用，而 OHOS 的文件选择器必须在主线程运行，使用 `napi_threadsafe_function` 实现线程安全回调。

### 2. 文件沙箱

OHOS 的文件选择器返回的是临时 URI，PPSSPP 无法直接访问。因此选择的文件会被复制到应用沙箱目录：

```typescript
const destPath = `${this.context.filesDir}/${fileName}`;
```

### 3. 文件夹权限

文件夹选择使用 `DocumentSelectMode.FOLDER`，返回的路径可能需要持久化权限才能长期访问。

## 相关文件

- `ohos/entry/src/main/cpp/napi/napi_ppsspp.cpp` - C++ 线程安全函数
- `ohos/entry/src/main/cpp/ohos_system_properties.cpp` - System_MakeRequest 处理
- `ohos/entry/src/main/ets/pages/Index.ets` - ArkTS 文件选择器
- `ohos/entry/src/main/ets/types/libppsspp.d.ts` - TypeScript 类型声明

## 测试方法

1. 启动 PPSSPP
2. 点击"浏览"按钮选择游戏
3. 应该弹出系统文件选择器
4. 选择文件后，文件会被复制到沙箱并加载
