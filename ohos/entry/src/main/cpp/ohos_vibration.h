// PPSSPP - OpenHarmony Platform Adaptation
// Copyright (c) 2024-2025 richshaw
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License 2.0 for more details.
//
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

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
