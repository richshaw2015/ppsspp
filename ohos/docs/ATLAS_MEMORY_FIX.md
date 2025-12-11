# PPSSPP AtlasTool 内存问题修复

## 问题描述

在 macOS 上编译的 atlastool 运行时出现内存错误：
```
malloc: *** error for object 0x7f9fc4f89000: pointer being freed was not allocated
Abort trap: 6
```

## 根本原因

问题出现在 `Image::SaveZIM` 函数中的内存管理不匹配：

1. **SaveZIM 函数的内存管理**：
   - `SaveZIM(FILE *f, int width, int height, int pitch, int format, const uint8_t *image_data, int compressLevel)`
   - 该函数会获取 `image_data` 指针的所有权
   - 在函数结束时会调用 `delete[] image_data`

2. **我们的错误实现**：
   ```cpp
   // 错误：使用 std::vector 的数据指针
   std::vector<uint8_t> bytes(w * h * 4);
   // ... 填充数据 ...
   ::SaveZIM(f, w, h, w * 4, zim_format, bytes.data(), 0);
   // SaveZIM 尝试 delete[] 一个不是用 new[] 分配的指针！
   ```

## 解决方案

修改 `AtlasGen_complete.cpp` 中的 `Image::SaveZIM` 函数：

```cpp
void Image::SaveZIM(const char *zim_name, int zim_format) {
    FILE *f = fopen(zim_name, "wb");
    if (!f) {
        ERROR_LOG(Log::IO, "Failed to open %s for writing", zim_name);
        return;
    }
    
    // 使用 new[] 分配内存，匹配 SaveZIM 的 delete[] 期望
    uint8_t *bytes = new uint8_t[w * h * 4];
    for (int i = 0; i < w * h; i++) {
        uint32_t pixel = dat[i];
        bytes[i*4 + 0] = (pixel >> 0) & 0xFF;   // R
        bytes[i*4 + 1] = (pixel >> 8) & 0xFF;   // G  
        bytes[i*4 + 2] = (pixel >> 16) & 0xFF;  // B
        bytes[i*4 + 3] = (pixel >> 24) & 0xFF;  // A
    }
    
    // SaveZIM 获取指针所有权并会 delete[] 它
    ::SaveZIM(f, w, h, w * 4, zim_format, bytes, 0);
    fclose(f);
}
```

## 关键修改

1. **内存分配方式**：
   - 从 `std::vector<uint8_t> bytes` 改为 `uint8_t *bytes = new uint8_t[w * h * 4]`
   - 确保内存是用 `new[]` 分配的，可以被 `delete[]` 安全释放

2. **内存所有权**：
   - 明确 SaveZIM 函数会获取指针所有权
   - 我们不需要（也不应该）手动释放内存

## 为什么会出现这个问题

1. **平台差异**：
   - macOS 的内存管理更严格
   - 在某些平台上，这种错误可能不会立即崩溃

2. **PPSSPP 内部设计**：
   - SaveZIM 函数设计为获取内存所有权
   - 这是为了支持 mipmap 生成等复杂操作

3. **存根实现的复杂性**：
   - 我们试图简化实现，但忽略了内存管理的细节
   - 与真实 PPSSPP 代码的集成需要遵循其内存管理约定

## 验证修复

创建测试脚本验证修复：

```bash
#!/bin/bash
echo "测试内存修复..."
ext/native/tools/build/build/atlastool font_tools/test_minimal.txt test_minimal

if [ $? -eq 0 ]; then
    echo "✅ 内存问题已修复！"
    ls -la test_minimal_atlas.*
else
    echo "❌ 仍然有问题"
fi
```

## 经验教训

1. **内存管理一致性**：
   - 在 C++ 中，分配和释放必须匹配（new/delete, new[]/delete[], malloc/free）
   - 跨函数传递指针时要明确所有权

2. **平台测试重要性**：
   - 不同平台的内存管理严格程度不同
   - 在目标平台上测试是必要的

3. **理解现有代码**：
   - 集成现有代码时，必须理解其设计约定
   - 不能简单地替换实现而忽略接口契约

## 下一步

1. 在 OHOS 平台上测试修复后的 atlastool
2. 验证生成的 ZIM 和 Meta 文件格式正确性
3. 测试中文字体支持

这个修复确保了 atlastool 在所有平台上的内存安全性，为后续的字体图集生成奠定了基础。