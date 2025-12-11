# 鸿蒙 HiLog 日志格式说明

## 隐私保护机制

鸿蒙系统的 HiLog 有隐私保护机制，默认会过滤日志中的敏感信息（数值、字符串等），显示为 `<private>`。

## 使用 %{public} 格式

要显示实际的数值，需要在格式字符串中使用 `%{public}` 前缀：

### 整数
```cpp
// 错误 - 会显示 <private>
OHOS_LOGI(TAG, "value: %d", 123);

// 正确 - 会显示实际数值
OHOS_LOGI(TAG, "value: %{public}d", 123);
```

### 浮点数
```cpp
// 错误
OHOS_LOGI(TAG, "value: %.2f", 3.14);

// 正确
OHOS_LOGI(TAG, "value: %{public}.2f", 3.14);
```

### 字符串
```cpp
// 错误
OHOS_LOGI(TAG, "name: %s", "test");

// 正确
OHOS_LOGI(TAG, "name: %{public}s", "test");
```

### 指针
```cpp
// 错误
OHOS_LOGI(TAG, "ptr: %p", ptr);

// 正确
OHOS_LOGI(TAG, "ptr: %{public}p", ptr);
```

## 常用格式对照表

| 类型 | 错误格式 | 正确格式 |
|------|---------|---------|
| int | %d | %{public}d |
| long | %ld | %{public}ld |
| float | %f | %{public}f |
| double | %.2f | %{public}.2f |
| string | %s | %{public}s |
| pointer | %p | %{public}p |
| unsigned | %u | %{public}u |
| hex | %x | %{public}x |

## 示例

### 触摸事件日志
```cpp
OHOS_LOGI(TAG, "Touch: x=%{public}d y=%{public}d", x, y);
```

### 显示参数日志
```cpp
OHOS_LOGI(TAG, "Display: %{public}dx%{public}d, dpi=%{public}.2f", 
          width, height, dpi);
```

### 混合格式
```cpp
OHOS_LOGI(TAG, "Surface size: %{public}dx%{public}d", 
          g_surfaceWidth, g_surfaceHeight);
```

## 注意事项

1. **调试日志**：开发阶段建议使用 `%{public}`，方便查看实际数值
2. **发布版本**：发布时可以移除 `%{public}`，保护用户隐私
3. **性能影响**：使用 `%{public}` 不会影响性能
4. **字符串常量**：纯字符串常量不需要 `%{public}`

## 自动转换工具

我们提供了一个 Python 脚本来自动为所有 OHOS_LOG 调用添加 `%{public}` 标记：

```bash
# 预览将要修改的内容（不实际修改文件）
python3 ohos/add_public_to_logs.py --dry-run

# 实际修改文件
python3 ohos/add_public_to_logs.py

# 修改指定文件
python3 ohos/add_public_to_logs.py ohos/entry/src/main/cpp/ohos_xcomponent.cpp
```

脚本会自动处理以下格式：
- `%d` → `%{public}d`
- `%f` → `%{public}f`
- `%.2f` → `%{public}.2f`
- `%s` → `%{public}s`
- `%p` → `%{public}p`
- 等等...

## 已修改的文件

以下文件已通过自动转换工具添加 `%{public}` 标记：

- `ohos/entry/src/main/cpp/ohos_xcomponent.cpp`
- `ohos/entry/src/main/cpp/ohos_audio.cpp`
- `ohos/entry/src/main/cpp/ohos_system.cpp`
- `ohos/entry/src/main/cpp/ohos_input.cpp`
- `ohos/entry/src/main/cpp/ohos_app.cpp`
- `ohos/entry/src/main/cpp/napi/napi_ppsspp.cpp`
- `ohos/entry/src/main/cpp/napi/napi_init.cpp`

## 参考文档

- [HiLog 开发指南](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/hilog-guidelines-0000001478061725-V3)
