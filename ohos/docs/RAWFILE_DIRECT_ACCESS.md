# OHOS Rawfile 直接访问方案

## 问题分析

你说得对！Android 的 assets 也是只读的，但可以通过 `AssetManager` 直接读取，不需要复制到文件系统。

OHOS 也提供了类似的机制：**Native Rawfile API**

## Android 的实现

Android 使用 `AAssetManager` 直接读取 assets：

```cpp
// Android
AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
AAsset* asset = AAssetManager_open(mgr, "file.txt", AASSET_MODE_BUFFER);
const void* data = AAsset_getBuffer(asset);
```

## OHOS 的 Native Rawfile API

OHOS 提供了类似的 API：

```cpp
#include <rawfile/raw_file_manager.h>

// 获取 ResourceManager
NativeResourceManager* nativeManager = OH_ResourceManager_InitNativeResourceManager(env, jsResMgr);

// 打开 rawfile
RawFile* rawFile = OH_ResourceManager_OpenRawFile(nativeManager, "assets/file.txt");

// 读取数据
long size = OH_ResourceManager_GetRawFileSize(rawFile);
char* buffer = new char[size];
int bytesRead = OH_ResourceManager_ReadRawFile(rawFile, buffer, size);

// 关闭文件
OH_ResourceManager_CloseRawFile(rawFile);

// 释放 ResourceManager
OH_ResourceManager_ReleaseNativeResourceManager(nativeManager);
```

## 实现方案

### 1. 创建 RawfileReader 类

类似于 Android 的 `AssetReader`，创建一个 `RawfileReader` 来实现 VFS 接口：

```cpp
// ohos_rawfile_reader.h
class RawfileReader : public VFSBackend {
public:
    RawfileReader(NativeResourceManager* mgr);
    ~RawfileReader();
    
    // VFS 接口实现
    bool GetFileListing(const std::string &path, std::vector<File::FileInfo> *listing, const char *filter) override;
    bool GetFileInfo(const std::string &path, File::FileInfo *info) override;
    std::string toString() const override { return "RawfileReader"; }
    
    // 文件操作
    u8 *ReadFile(const char *path, size_t *size) override;
    bool ReadFileToString(const char *path, std::string *contents) override;
    
private:
    NativeResourceManager* resourceManager_;
};
```

### 2. 在 NAPI 中传递 ResourceManager

```cpp
// napi_init.cpp
static napi_value InitEmulator(napi_env env, napi_callback_info info) {
    // 获取 ResourceManager
    napi_value jsResMgr;
    napi_get_named_property(env, args[2], "resourceManager", &jsResMgr);
    
    // 转换为 Native ResourceManager
    NativeResourceManager* nativeResMgr = OH_ResourceManager_InitNativeResourceManager(env, jsResMgr);
    
    // 保存到全局变量
    g_ohosResourceManager = nativeResMgr;
    
    // 注册 VFS
    g_VFS.Register("", new RawfileReader(nativeResMgr));
}
```

### 3. 在 ArkTS 中传递 ResourceManager

```typescript
// Index.ets
ppsspp.initEmulator(
    this.context.filesDir, 
    this.context.cacheDir,
    this.context.resourceManager  // 传递 ResourceManager
);
```

## 优势

### 相比当前方案（复制文件）

| 特性 | 当前方案（复制） | 直接读取方案 |
|------|-----------------|-------------|
| 首次启动时间 | 慢（需要复制 90+ 文件） | 快（无需复制） |
| 存储空间 | 占用双倍空间 | 只占用 HAP 空间 |
| 维护成本 | 高（需要维护文件列表） | 低（自动读取） |
| 文件遗漏风险 | 高 | 无 |
| 实现复杂度 | 低 | 中 |

### 相比 Android

| 特性 | Android | OHOS（直接读取） |
|------|---------|-----------------|
| API | AAssetManager | OH_ResourceManager |
| 文件访问 | 直接读取 | 直接读取 |
| 目录列表 | 支持 | **不支持** |
| 性能 | 快 | 快 |

## 限制

### OHOS 的限制

**无法列出目录内容**

OHOS 的 rawfile API 没有提供列出目录的功能：

```cpp
// ❌ 不存在这样的 API
OH_ResourceManager_ListRawFiles(mgr, "assets");
```

这意味着：
- 无法实现 `GetFileListing()` 接口
- PPSSPP 的某些功能可能依赖目录列表（如语言文件扫描）

### 解决方案

#### 方案 A：混合方案

- **核心文件**：通过 rawfile 直接读取
- **目录列表**：预先生成并嵌入代码

```cpp
// 预先定义的文件列表
static const char* LANG_FILES[] = {
    "lang/zh_CN.ini",
    "lang/en_US.ini",
    "lang/ja_JP.ini",
    // ...
};

bool RawfileReader::GetFileListing(const std::string &path, ...) {
    if (path == "lang") {
        // 返回预定义的语言文件列表
        for (const char* file : LANG_FILES) {
            listing->push_back(FileInfo(file));
        }
        return true;
    }
    return false;
}
```

#### 方案 B：元数据文件

在 rawfile 中包含一个元数据文件，列出所有文件：

```
assets/
  _file_list.txt  # 包含所有文件的列表
  font_atlas.zim
  lang/zh_CN.ini
  ...
```

```cpp
bool RawfileReader::GetFileListing(const std::string &path, ...) {
    // 读取 _file_list.txt
    std::string fileList;
    ReadFileToString("_file_list.txt", &fileList);
    
    // 解析并过滤
    // ...
}
```

## 实现步骤

### 1. 添加 rawfile 依赖

在 `CMakeLists.txt` 中：

```cmake
target_link_libraries(ppsspp_ohos
    rawfile.z  # Rawfile API
)
```

### 2. 创建 RawfileReader

```cpp
// ohos/entry/src/main/cpp/ohos_rawfile_reader.h
// ohos/entry/src/main/cpp/ohos_rawfile_reader.cpp
```

### 3. 修改 NAPI 接口

传递 ResourceManager 到 Native 层。

### 4. 注册 VFS

在 `NativeInit` 中注册 `RawfileReader`。

### 5. 生成文件列表元数据

```bash
find assets -type f > assets/_file_list.txt
```

## 性能对比

### 当前方案（复制文件）

```
首次启动：
  1. 复制 90+ 文件：~3-5 秒
  2. 占用存储：~15 MB（双倍）

后续启动：
  1. 检查文件：~0.1 秒
  2. 读取文件：从文件系统（快）
```

### 直接读取方案

```
首次启动：
  1. 无需复制：~0 秒
  2. 占用存储：~7.5 MB（单份）

后续启动：
  1. 无需检查：~0 秒
  2. 读取文件：从 rawfile（快）
```

## 推荐方案

### 短期：保持当前方案

- ✅ 已经实现并工作
- ✅ 简单可靠
- ❌ 首次启动慢
- ❌ 占用双倍空间

### 长期：实现直接读取

- ✅ 首次启动快
- ✅ 节省存储空间
- ✅ 无需维护文件列表
- ❌ 需要实现 RawfileReader
- ❌ 需要处理目录列表问题

## 参考资料

### OHOS Rawfile API

- `OH_ResourceManager_InitNativeResourceManager`
- `OH_ResourceManager_OpenRawFile`
- `OH_ResourceManager_ReadRawFile`
- `OH_ResourceManager_GetRawFileSize`
- `OH_ResourceManager_CloseRawFile`
- `OH_ResourceManager_ReleaseNativeResourceManager`

### PPSSPP VFS 接口

- `VFSBackend`
- `DirectoryReader`
- `g_VFS.Register()`

## 总结

你的观察是正确的！OHOS 完全可以像 Android 一样直接读取 rawfile，不需要复制。

**当前方案的问题**：
- 首次启动慢
- 占用双倍空间
- 需要手动维护文件列表
- 容易遗漏文件（如音效文件）

**直接读取方案的优势**：
- 首次启动快
- 节省存储空间
- 无需维护文件列表
- 不会遗漏文件

**唯一的挑战**：
- OHOS 不支持列出目录
- 需要预先生成文件列表或使用元数据文件

建议：
1. **短期**：修复当前方案（添加音效文件）
2. **长期**：实现 RawfileReader，直接读取 rawfile

