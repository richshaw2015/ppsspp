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
 * 鸿蒙音频后端接口
 * 参考 android/jni/AndroidAudio.h
 */

#ifndef OHOS_AUDIO_H
#define OHOS_AUDIO_H

#include <string>

// 音频回调函数类型（与 Android 兼容）
// buffer: 音频缓冲区（16-bit PCM）
// numSamples: 样本数（不是帧数！stereo 时 numSamples = numFrames * 2）
// sampleRateHz: 采样率
typedef void (*OhosAudioCallback)(short *buffer, int numSamples, int sampleRateHz);

// 音频状态结构体（不透明）
struct OhosAudioState;

/**
 * 初始化音频系统
 * @param callback 音频回调函数
 * @param optimalFramesPerBuffer 最佳缓冲区大小（帧数），0 表示使用默认值
 * @param optimalSampleRate 最佳采样率，0 表示使用默认值
 * @return 音频状态对象，失败返回 nullptr
 */
OhosAudioState *OhosAudio_Init(OhosAudioCallback callback, int optimalFramesPerBuffer, int optimalSampleRate);

/**
 * 暂停音频（应用进入后台时调用）
 */
bool OhosAudio_Pause(OhosAudioState *state);

/**
 * 恢复音频（应用回到前台时调用）
 */
bool OhosAudio_Resume(OhosAudioState *state);

/**
 * 关闭音频系统
 */
bool OhosAudio_Shutdown(OhosAudioState *state);

/**
 * 获取错误信息
 */
const std::string OhosAudio_GetErrorString(OhosAudioState *state);

/**
 * 录音相关（暂不实现）
 */
bool OhosAudio_Recording_SetSampleRate(OhosAudioState *state, int sampleRate);
bool OhosAudio_Recording_Start(OhosAudioState *state);
bool OhosAudio_Recording_Stop(OhosAudioState *state);
bool OhosAudio_Recording_State(OhosAudioState *state);

#endif // OHOS_AUDIO_H
