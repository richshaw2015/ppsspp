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
 * 鸿蒙音频后端实现
 * 参考 android/jni/AndroidAudio.cpp 和 OpenSLContext.cpp
 * 使用 OHAudio API (类似于 OpenSL ES)
 */

#include "ohos_audio.h"
#include "ohos_hilog.h"
#include "Common/Log.h"
#include <ohaudio/native_audiostreambuilder.h>
#include <ohaudio/native_audiorenderer.h>
#include <mutex>
#include <cstring>

#define AUDIO_TAG "PPSSPP_Audio"

// 音频上下文类
class OhosAudioContext {
public:
    OhosAudioContext(OhosAudioCallback cb, int framesPerBuffer, int sampleRate);
    ~OhosAudioContext();
    
    bool Init();
    void Shutdown();
    
    int GetSampleRate() const { return sampleRate_; }
    int GetFramesPerBuffer() const { return framesPerBuffer_; }
    
private:
    // 音频回调（从 OHAudio 调用）
    static int32_t AudioRendererCallback(
        OH_AudioRenderer* renderer,
        void* userData,
        void* audioData,
        int32_t audioDataSize);
    
    OhosAudioCallback callback_;
    int framesPerBuffer_;
    int sampleRate_;
    
    OH_AudioStreamBuilder* builder_;
    OH_AudioRenderer* renderer_;
    
    std::mutex errorMutex_;
    std::string errorString_;
};

// 音频状态结构体
struct OhosAudioState {
    OhosAudioContext *ctx = nullptr;
    OhosAudioCallback callback = nullptr;
    int frames_per_buffer = 0;
    int sample_rate = 0;
    
    // 录音相关（暂不实现）
    int input_enable = 0;
    int input_sample_rate = 0;
};

// ============================================================================
// OhosAudioContext 实现
// ============================================================================

OhosAudioContext::OhosAudioContext(OhosAudioCallback cb, int framesPerBuffer, int sampleRate)
    : callback_(cb)
    , framesPerBuffer_(framesPerBuffer)
    , sampleRate_(sampleRate)
    , builder_(nullptr)
    , renderer_(nullptr) {
    
    // 限制缓冲区大小
    if (framesPerBuffer_ == 0)
        framesPerBuffer_ = 512;
    if (framesPerBuffer_ < 64)
        framesPerBuffer_ = 64;
    if (framesPerBuffer_ > 4096)
        framesPerBuffer_ = 4096;
    
    // 限制采样率
    if (sampleRate_ == 0)
        sampleRate_ = 44100;
    
    OHOS_LOGI(AUDIO_TAG, "OhosAudioContext created: frames=%{public}d, rate=%{public}d", 
              framesPerBuffer_, sampleRate_);
}

OhosAudioContext::~OhosAudioContext() {
    Shutdown();
}

bool OhosAudioContext::Init() {
    OHOS_LOGI(AUDIO_TAG, "Initializing OHAudio renderer...");
    
    // 1. 创建 AudioStreamBuilder
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder_, AUDIOSTREAM_TYPE_RENDERER);
    if (result != AUDIOSTREAM_SUCCESS || !builder_) {
        ERROR_LOG(Log::Audio, "Failed to create AudioStreamBuilder: %d", result);
        errorString_ = "Failed to create AudioStreamBuilder";
        return false;
    }
    
    // 2. 配置音频参数
    OH_AudioStreamBuilder_SetSamplingRate(builder_, sampleRate_);
    OH_AudioStreamBuilder_SetChannelCount(builder_, 2);  // 立体声
    OH_AudioStreamBuilder_SetSampleFormat(builder_, AUDIOSTREAM_SAMPLE_S16LE);  // 16-bit PCM
    OH_AudioStreamBuilder_SetEncodingType(builder_, AUDIOSTREAM_ENCODING_TYPE_RAW);
    OH_AudioStreamBuilder_SetLatencyMode(builder_, AUDIOSTREAM_LATENCY_MODE_FAST);  // 低延迟模式
    
    // 3. 设置回调函数（使用高级回调）
    OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced(builder_, AudioRendererCallback, this);
    
    // 4. 创建 AudioRenderer
    result = OH_AudioStreamBuilder_GenerateRenderer(builder_, &renderer_);
    if (result != AUDIOSTREAM_SUCCESS || !renderer_) {
        ERROR_LOG(Log::Audio, "Failed to create AudioRenderer: %d", result);
        errorString_ = "Failed to create AudioRenderer";
        OH_AudioStreamBuilder_Destroy(builder_);
        builder_ = nullptr;
        return false;
    }
    
    // 5. 启动音频流
    result = OH_AudioRenderer_Start(renderer_);
    if (result != AUDIOSTREAM_SUCCESS) {
        ERROR_LOG(Log::Audio, "Failed to start AudioRenderer: %d", result);
        errorString_ = "Failed to start AudioRenderer";
        OH_AudioRenderer_Release(renderer_);
        renderer_ = nullptr;
        OH_AudioStreamBuilder_Destroy(builder_);
        builder_ = nullptr;
        return false;
    }
    
    INFO_LOG(Log::Audio, "OHAudio renderer initialized successfully");
    OHOS_LOGI(AUDIO_TAG, "OHAudio renderer started");
    return true;
}

void OhosAudioContext::Shutdown() {
    if (renderer_) {
        OHOS_LOGI(AUDIO_TAG, "Stopping AudioRenderer...");
        OH_AudioRenderer_Stop(renderer_);
        OH_AudioRenderer_Release(renderer_);
        renderer_ = nullptr;
    }
    
    if (builder_) {
        OH_AudioStreamBuilder_Destroy(builder_);
        builder_ = nullptr;
    }
    
    OHOS_LOGI(AUDIO_TAG, "OHAudio renderer shut down");
}

int32_t OhosAudioContext::AudioRendererCallback(
    OH_AudioRenderer* renderer,
    void* userData,
    void* audioData,
    int32_t audioDataSize) {
    
    OhosAudioContext* ctx = static_cast<OhosAudioContext*>(userData);
    if (!ctx || !ctx->callback_) {
        // 没有回调，填充静音
        memset(audioData, 0, audioDataSize);
        return audioDataSize;  // 返回写入的字节数
    }
    
    // 计算样本数（stereo: numSamples = numFrames * 2）
    int numFrames = audioDataSize / (2 * sizeof(int16_t));  // 2 channels, 16-bit
    int numSamples = numFrames * 2;
    
    // 调用 PPSSPP 的音频回调
    ctx->callback_(static_cast<short*>(audioData), numSamples, ctx->sampleRate_);
    
    return audioDataSize;  // 返回写入的字节数
}

// ============================================================================
// C API 实现（与 Android 兼容）
// ============================================================================

static std::string g_error;
static std::mutex g_errorMutex;

OhosAudioState *OhosAudio_Init(OhosAudioCallback callback, int optimalFramesPerBuffer, int optimalSampleRate) {
    OHOS_LOGI(AUDIO_TAG, "OhosAudio_Init: frames=%{public}d, rate=%{public}d", 
              optimalFramesPerBuffer, optimalSampleRate);
    
    OhosAudioState *state = new OhosAudioState();
    state->callback = callback;
    state->frames_per_buffer = optimalFramesPerBuffer ? optimalFramesPerBuffer : 512;
    state->sample_rate = optimalSampleRate ? optimalSampleRate : 44100;
    
    INFO_LOG(Log::Audio, "OhosAudio_Init: frames=%d, rate=%d", 
             state->frames_per_buffer, state->sample_rate);
    
    return state;
}

bool OhosAudio_Resume(OhosAudioState *state) {
    if (!state) {
        ERROR_LOG(Log::Audio, "Audio was shutdown, cannot resume!");
        return false;
    }
    
    if (!state->ctx) {
        INFO_LOG(Log::Audio, "Creating OhosAudioContext...");
        OHOS_LOGI(AUDIO_TAG, "Creating audio context...");
        
        state->ctx = new OhosAudioContext(state->callback, state->frames_per_buffer, state->sample_rate);
        
        bool init_retval = state->ctx->Init();
        if (!init_retval) {
            ERROR_LOG(Log::Audio, "Failed to initialize OhosAudioContext");
            delete state->ctx;
            state->ctx = nullptr;
            return false;
        }
        
        INFO_LOG(Log::Audio, "OhosAudioContext created successfully");
        return true;
    }
    
    return false;
}

bool OhosAudio_Pause(OhosAudioState *state) {
    if (!state) {
        ERROR_LOG(Log::Audio, "Audio was shutdown, cannot pause!");
        return false;
    }
    
    if (state->ctx) {
        INFO_LOG(Log::Audio, "Destroying OhosAudioContext...");
        OHOS_LOGI(AUDIO_TAG, "Pausing audio...");
        
        delete state->ctx;
        state->ctx = nullptr;
        
        INFO_LOG(Log::Audio, "OhosAudioContext destroyed");
        return true;
    }
    
    return false;
}

bool OhosAudio_Shutdown(OhosAudioState *state) {
    if (!state) {
        ERROR_LOG(Log::Audio, "Audio already shutdown!");
        return false;
    }
    
    if (state->ctx) {
        ERROR_LOG(Log::Audio, "Should not shut down when playing! Something is wrong!");
        return false;
    }
    
    delete state;
    INFO_LOG(Log::Audio, "OhosAudio completely unloaded.");
    OHOS_LOGI(AUDIO_TAG, "Audio shutdown complete");
    return true;
}

const std::string OhosAudio_GetErrorString(OhosAudioState *state) {
    if (!state) {
        return "No state";
    }
    std::unique_lock<std::mutex> lock(g_errorMutex);
    return g_error;
}

// 录音相关（暂不实现）
bool OhosAudio_Recording_SetSampleRate(OhosAudioState *state, int sampleRate) {
    if (!state) {
        ERROR_LOG(Log::Audio, "OhosAudioState not initialized, cannot set recording sample rate");
        return false;
    }
    state->input_sample_rate = sampleRate;
    INFO_LOG(Log::Audio, "OhosAudio_Recording_SetSampleRate=%d (not implemented)", sampleRate);
    return true;
}

bool OhosAudio_Recording_Start(OhosAudioState *state) {
    if (!state) {
        ERROR_LOG(Log::Audio, "OhosAudioState not initialized, cannot start recording!");
        return false;
    }
    INFO_LOG(Log::Audio, "OhosAudio_Recording_Start (not implemented)");
    return false;
}

bool OhosAudio_Recording_Stop(OhosAudioState *state) {
    if (!state) {
        ERROR_LOG(Log::Audio, "OhosAudioState not initialized, cannot stop recording!");
        return false;
    }
    INFO_LOG(Log::Audio, "OhosAudio_Recording_Stop (not implemented)");
    return false;
}

bool OhosAudio_Recording_State(OhosAudioState *state) {
    if (!state) {
        return false;
    }
    return state->input_enable;
}
