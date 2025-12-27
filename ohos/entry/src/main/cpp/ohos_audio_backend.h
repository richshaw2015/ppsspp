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
 * 鸿蒙音频后端 - AudioBackend 接口实现
 * 参考 Common/Audio/OpenSLESAudioBackend.h
 */

#pragma once

#include "Common/Audio/AudioBackend.h"
#include "ohos_audio.h"
#include <mutex>

class OhosAudioBackend : public AudioBackend {
public:
    OhosAudioBackend();
    ~OhosAudioBackend() override;
    
    void EnumerateDevices(std::vector<AudioDeviceDesc> *outputDevices, bool captureDevices = false) override;
    void SetRenderCallback(RenderCallback callback, void *userdata) override;
    bool InitOutputDevice(std::string_view uniqueId, LatencyMode latencyMode, bool *revertedToDefault) override;
    int SampleRate() const override;
    int BufferSize() const override;
    int PeriodFrames() const override;
    void DescribeOutputFormat(char *buffer, size_t bufferSize) const override;
    void FrameUpdate(bool allowAutoChange) override;
    
private:
    // 静态回调函数（从 OhosAudio 调用）
    static void AudioCallbackStatic(short *buffer, int numSamples, int sampleRateHz);
    
    // 实例回调函数
    void AudioCallback(short *buffer, int numSamples, int sampleRateHz);
    
    RenderCallback renderCallback_ = nullptr;
    void *renderUserdata_ = nullptr;
    
    OhosAudioState *audioState_ = nullptr;
    
    int sampleRate_ = 44100;
    int framesPerBuffer_ = 512;
    
    std::mutex mutex_;
    
    // 用于 float 到 short 的转换
    float *mixBuffer_ = nullptr;
    int mixBufferSize_ = 0;
};
