/**
 * OHOS HiLog 包装器
 * 避免与 PPSSPP 的 LogLevel 类型冲突
 * 
 * 问题：OHOS 的 hilog/log.h 定义了 LogLevel 枚举
 *       PPSSPP 的 Common/Log.h 也定义了 LogLevel 枚举
 *       两者冲突导致编译错误
 * 
 * 解决方案：不包含 hilog/log.h，直接声明需要的函数
 */

#ifndef OHOS_HILOG_H
#define OHOS_HILOG_H

#ifdef __cplusplus
extern "C" {
#endif

// HiLog 级别常量（与 hilog/log.h 中的值相同）
#define OHOS_LOG_DEBUG 3
#define OHOS_LOG_INFO  4
#define OHOS_LOG_WARN  5
#define OHOS_LOG_ERROR 6
#define OHOS_LOG_FATAL 7

// HiLog 类型常量
#define OHOS_LOG_APP 0

// 日志域
#define OHOS_LOG_DOMAIN 0x0001

// 声明 OH_LOG_Print 函数（来自 libhilog_ndk.z.so）
int OH_LOG_Print(int type, int level, unsigned int domain, const char *tag, const char *fmt, ...)
    __attribute__((__format__(printf, 5, 6)));

// 自定义日志宏
// 注意：鸿蒙的隐私保护机制会过滤日志中的数值
// 需要使用 %{public}d, %{public}f, %{public}s 等格式来显示数值
#define OHOS_LOGD(tag, fmt, ...) \
    OH_LOG_Print(OHOS_LOG_APP, OHOS_LOG_DEBUG, OHOS_LOG_DOMAIN, tag, fmt, ##__VA_ARGS__)

#define OHOS_LOGI(tag, fmt, ...) \
    OH_LOG_Print(OHOS_LOG_APP, OHOS_LOG_INFO, OHOS_LOG_DOMAIN, tag, fmt, ##__VA_ARGS__)

#define OHOS_LOGW(tag, fmt, ...) \
    OH_LOG_Print(OHOS_LOG_APP, OHOS_LOG_WARN, OHOS_LOG_DOMAIN, tag, fmt, ##__VA_ARGS__)

#define OHOS_LOGE(tag, fmt, ...) \
    OH_LOG_Print(OHOS_LOG_APP, OHOS_LOG_ERROR, OHOS_LOG_DOMAIN, tag, fmt, ##__VA_ARGS__)

// 便捷宏：自动添加 public 标记的日志宏（用于调试）
// 使用示例：OHOS_LOGI_PUB(tag, "value: %d", 123) 会自动转换为 "value: %{public}d"
// 但由于 C 预处理器的限制，这个功能很难实现，建议直接在代码中使用 %{public}d

#ifdef __cplusplus
}
#endif

#endif // OHOS_HILOG_H
