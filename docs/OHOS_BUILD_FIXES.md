# OHOS 编译错误修复记录

## 修复时间
2025-12-13

## 问题 1: ArkTS 编译错误

### 错误信息
```
ERROR: 10605008 ArkTS Compiler Error
Error Message: Use explicit types instead of "any", "unknown" (arkts-no-any-unknown)

ERROR: 10505001 ArkTS Compiler Error
Error Message: Argument of type 'number' is not assignable to parameter of type 'string'.

ERROR: 10505001 ArkTS Compiler Error
Error Message: Property 'region' does not exist on type 'Configuration'.
```

### 原因分析
1. ArkTS 要求所有变量都有显式类型注解
2. `deviceInfo.sdkApiVersion` 是 number 类型，需要转换为 string 才能 parseInt
3. HarmonyOS 的 `Configuration` 类型没有 `region` 属性

### 解决方案

#### 文件：`EntryAbility.ets`

**修复 1: 添加显式类型注解**
```typescript
// ❌ 之前
const deviceName = deviceInfo.productModel || "HarmonyOS Device";
const osVersion = parseInt(deviceInfo.sdkApiVersion) || 5;

// ✅ 现在
const deviceName: string = deviceInfo.productModel || "HarmonyOS Device";
const osVersion: number = parseInt(deviceInfo.sdkApiVersion.toString()) || 5;
```

**修复 2: 使用语言代码推断区域**
```typescript
// ❌ 之前 - Configuration 没有 region 属性
const systemRegion = this.context.config.region || "CN";

// ✅ 现在 - 根据语言推断区域
const systemLanguage: string = this.context.config.language || "zh";
const systemRegion: string = systemLanguage === "zh" ? "CN" : "US";
const langRegion: string = `${systemLanguage}_${systemRegion}`;
```

**修复 3: 所有变量添加类型**
```typescript
const deviceName: string = ...
const deviceBuild: string = ...
const osVersion: number = ...
const systemLanguage: string = ...
const systemRegion: string = ...
const langRegion: string = ...
const xres: number = ...
const yres: number = ...
const dpi: number = ...
const refreshRate: number = ...
let devType: number = 0;
```

## 问题 2: C++ 链接错误

### 错误信息
```
ld.lld: error: duplicate symbol: System_GetPropertyInt(SystemProperty)
>>> defined at ohos_system_properties.cpp:145
>>> defined at ohos_system_stubs.cpp:42

ld.lld: error: duplicate symbol: System_GetPropertyStringVec(SystemProperty)
>>> defined at ohos_system_properties.cpp:133
>>> defined at ohos_system_stubs.cpp:166
```

### 原因分析
在重构系统属性实现时，`ohos_system_properties.cpp` 中新增了完整的实现，但 `ohos_system_stubs.cpp` 中还保留着旧的桩实现，导致符号重复定义。

### 解决方案

#### 文件：`ohos_system_stubs.cpp`

**删除重复的函数定义**：

```cpp
// ❌ 删除这些函数（已在 ohos_system_properties.cpp 中实现）
int64_t System_GetPropertyInt(SystemProperty prop) {
    std::string value = System_GetProperty(prop);
    if (value.empty()) {
        return 0;
    }
    return std::stoll(value);
}

std::vector<std::string> System_GetPropertyStringVec(SystemProperty prop) {
    INFO_LOG(Log::System, "System_GetPropertyStringVec: %d (returning empty)", (int)prop);
    return std::vector<std::string>();
}
```

**添加注释说明**：
```cpp
// ============================================================================
// System 属性函数
// ============================================================================
// 注意：System_GetProperty, System_GetPropertyInt, System_GetPropertyFloat,
//       System_GetPropertyBool, System_GetPropertyStringVec
//       已在 ohos_system_properties.cpp 中实现
```

## 函数分布说明

### ohos_system_properties.cpp（新实现）
负责系统属性的完整实现，包括：
- ✅ `System_GetProperty()` - 字符串属性
- ✅ `System_GetPropertyInt()` - 整数属性
- ✅ `System_GetPropertyFloat()` - 浮点属性
- ✅ `System_GetPropertyBool()` - 布尔属性
- ✅ `System_GetPropertyStringVec()` - 字符串数组属性
- ✅ `OhosSystemProperties_Init()` - 初始化函数
- ✅ `OhosSystemProperties_SetSafeInsets()` - 设置安全区域
- ✅ `OhosSystemProperties_SetAudioConfig()` - 设置音频配置

### ohos_system_stubs.cpp（桩实现）
负责其他系统功能的桩实现，包括：
- ✅ `System_Notify()` - 系统通知
- ✅ `System_ShowKeyboard()` - 显示键盘
- ✅ `System_LaunchUrl()` - 打开 URL
- ✅ `System_Toast()` - 显示 Toast
- ✅ VR 相关函数（桩）

## 验证清单

### ArkTS 编译
- ✅ 所有变量都有显式类型注解
- ✅ 类型转换正确（number → string）
- ✅ 使用正确的 API（language 而不是 region）
- ✅ 无 `any` 或 `unknown` 类型

### C++ 链接
- ✅ 无重复符号定义
- ✅ 函数分布清晰
- ✅ 注释说明完整

### 功能测试
- ⬜ 系统属性正确初始化
- ⬜ 设备信息正确获取
- ⬜ 显示属性正确传递
- ⬜ 安全区域正确设置

## 相关文件

### 修改的文件
1. `ohos/entry/src/main/ets/entryability/EntryAbility.ets`
   - 添加显式类型注解
   - 修复 API 使用
   - 实现系统属性初始化

2. `ohos/entry/src/main/cpp/ohos_system_stubs.cpp`
   - 删除重复的函数定义
   - 添加注释说明

### 新增的文件
1. `ohos/entry/src/main/cpp/ohos_system_properties.cpp`
   - 完整的系统属性实现

2. `ohos/entry/src/main/cpp/ohos_system.h`
   - 初始化函数声明

3. `ohos/entry/src/main/cpp/napi/napi_init.cpp`
   - NAPI 绑定函数

## 编译命令

```bash
# 清理构建
hvigorw clean

# 重新构建
hvigorw assembleHap --mode module -p product=default
```

## 预期结果

编译成功，无错误和警告：
```
BUILD SUCCESSFUL in XXs
```

## 注意事项

1. **类型安全**：ArkTS 要求严格的类型检查，所有变量都必须有显式类型
2. **API 兼容性**：HarmonyOS 的 API 可能与 Android 不同，需要查阅官方文档
3. **符号冲突**：避免在多个文件中定义相同的函数
4. **注释维护**：在代码中添加清晰的注释，说明函数的实现位置

## 参考文档

- [OHOS_SYSTEM_PROPERTIES_IMPLEMENTATION.md](./OHOS_SYSTEM_PROPERTIES_IMPLEMENTATION.md)
- [OHOS_SYSTEM_PROPERTIES_REFACTOR.md](./OHOS_SYSTEM_PROPERTIES_REFACTOR.md)
- [OHOS_ADAPTATION_COMPLETED.md](./OHOS_ADAPTATION_COMPLETED.md)
