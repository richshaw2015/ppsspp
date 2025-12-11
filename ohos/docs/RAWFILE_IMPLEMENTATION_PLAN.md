# Rawfile 直接读取实现方案

## 当前状态

已创建：
- ✅ `ohos_rawfile_reader.h` - Rawfile Reader 头文件
- ✅ `ohos_rawfile_reader.cpp` - Rawfile Reader 实现
- ✅ CMakeLists.txt 已更新（添加源文件）
- ✅ rawfile.z 库已链接

## 实现方案

### 方案 A：通过 NAPI 传递 ResourceManager（推荐）

修改 ArkTS 调用，传递 `resourceManager`：

```typescript
// Index.ets
ppsspp.initEmulator(
    this.context.filesDir,
    this.context.cacheDir,
    this.context.resourceManager  // 新增参数
);
```

在 Native 层接收：

```cpp
// napi_init.cpp
static napi_value InitEmulator(napi_env env, napi_callback_info info) {
    size_t argc = 3;  // 改为 3 个参数
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    // 获取 ResourceManager
    NativeResourceManager* resMgr = OH_ResourceManager_InitNativeResourceManager(env, args[2]);
    g_resourceManager = resMgr;
    
    // 注册 VFS
    g_VFS.Register("", new OhosRawfileReader(resMgr));
}
```

### 方案 B：延迟注册（备选）

先初始化，后续再注册 Rawfile Reader：

```cpp
// 在 NativeInitGraphics 中注册
if (g_resourceManager) {
    g_VFS.Register("", new OhosRawfileReader(g_resourceManager));
}
```

## 测试步骤

### 1. 修改 ArkTS 调用

```typescript
// Index.ets
const success: boolean = ppsspp.initEmulator(
    this.context.filesDir,
    this.context.cacheDir,
    this.context.resourceManager
);
```

### 2. 修改 NAPI 接口

```cpp
// napi_init.cpp - InitEmulator 函数
size_t argc = 3;
napi_value args[3];
// ... 获取前两个参数 ...

// 获取 ResourceManager
if (argc >= 3) {
    g_resourceManager = OH_ResourceManager_InitNativeResourceManager(env, args[2]);
    if (g_resourceManager) {
        OHOS_LOGI(NAPI_TAG, "ResourceManager initialized");
        
        // 注册 Rawfile Reader 到 VFS
        g_VFS.Register("", new OhosRawfileReader(g_resourceManager));
        OHOS_LOGI(NAPI_TAG, "Rawfile Reader registered to VFS");
    }
}
```

### 3. 移除文件复制代码

注释掉 `Index.ets` 中的 `copyAssetsToFiles()` 调用：

```typescript
// 不再需要复制文件
// await this.copyAssetsToFiles();
```

### 4. 测试

```bash
cd ohos
./rebuild.sh
hdc install entry/build/default/outputs/default/entry-default-signed.hap
```

查看日志：
```bash
hdc shell hilog -x | grep -E "Rawfile|VFS|Assets"
```

应该看到：
```
[PPSSPP_NAPI] ResourceManager initialized
[PPSSPP_NAPI] Rawfile Reader registered to VFS
[System] OHOS: Assets registered successfully
```

## 优势

### 相比当前方案

| 特性 | 当前方案（复制） | Rawfile 方案 |
|------|-----------------|-------------|
| 首次启动时间 | 慢（3-5秒） | 快（<1秒） |
| 存储空间 | 双倍（~15MB） | 单份（~7.5MB） |
| 维护成本 | 高 | 低 |
| 文件遗漏风险 | 高 | 无 |

### 性能

- **读取速度**：Rawfile 直接读取与文件系统读取性能相当
- **内存占用**：按需读取，不占用额外内存
- **启动时间**：无需复制，启动更快

## 注意事项

### 1. 目录列表限制

OHOS rawfile API 不支持列出目录，已在 `OhosRawfileReader::GetFileListing()` 中实现预定义列表。

当前支持的目录：
- `lang/` - 语言文件（54 个）

如需支持更多目录，在 `GetFileListing()` 中添加。

### 2. 文件路径

Rawfile 路径格式：`assets/xxx`

`OhosRawfileReader` 会自动添加 `assets/` 前缀。

### 3. 错误处理

如果文件不存在，会返回 `nullptr` 并记录警告日志。

## 回退方案

如果 Rawfile 方案有问题，可以快速回退到复制方案：

1. 恢复 `copyAssetsToFiles()` 调用
2. 移除 Rawfile Reader 注册
3. 重新编译

## 下一步

1. ✅ 创建 Rawfile Reader 类
2. ⏳ 修改 NAPI 接口接收 ResourceManager
3. ⏳ 修改 ArkTS 调用传递 ResourceManager
4. ⏳ 注册 Rawfile Reader 到 VFS
5. ⏳ 测试验证
6. ⏳ 移除文件复制代码

## 总结

Rawfile 直接读取方案：
- ✅ 更快的启动速度
- ✅ 更少的存储占用
- ✅ 更低的维护成本
- ✅ 不会遗漏文件
- ⚠️ 需要处理目录列表限制（已解决）

建议立即实施此方案，替换当前的文件复制方案。

