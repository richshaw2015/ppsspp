/**
 * 鸿蒙震动功能接口
 * 提供震动功能的 C++ 接口，通过回调机制调用 ArkTS 层的震动 API
 */

#ifndef OHOS_VIBRATION_H
#define OHOS_VIBRATION_H

#include <functional>

namespace OhosVibration {

/**
 * 震动回调函数类型
 * @param duration 震动时长（毫秒）
 * @return 是否成功触发震动
 */
using VibrationCallback = std::function<bool(int duration)>;

/**
 * 初始化震动系统
 */
bool Initialize();

/**
 * 关闭震动系统
 */
void Shutdown();

/**
 * 设置震动回调函数（由 ArkTS 层调用）
 * @param callback 震动回调函数
 */
void SetVibrationCallback(VibrationCallback callback);

/**
 * 触发震动
 * @param duration 震动时长（毫秒）
 * @return 是否成功触发震动
 */
bool Vibrate(int duration);

/**
 * 检查是否支持震动
 */
bool IsVibrationSupported();

} // namespace OhosVibration

#endif // OHOS_VIBRATION_H