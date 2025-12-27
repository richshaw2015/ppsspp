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
 * 鸿蒙应用层接口
 */

#ifndef OHOS_APP_H
#define OHOS_APP_H

#include <string>

namespace OhosApp {

/**
 * 初始化应用
 */
bool Initialize();

/**
 * 关闭应用
 */
void Shutdown();

/**
 * 加载游戏
 * @param gamePath 游戏文件路径
 * @return 是否成功
 */
bool LoadGame(const std::string& gamePath);

/**
 * 运行一帧
 */
void RunFrame();

/**
 * 暂停模拟器
 */
void Pause();

/**
 * 恢复模拟器
 */
void Resume();

/**
 * 检查是否正在运行
 */
bool IsRunning();

} // namespace OhosApp

#endif // OHOS_APP_H
