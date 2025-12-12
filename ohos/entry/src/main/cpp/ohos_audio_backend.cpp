/**
 * 鸿蒙音频后端 - AudioBackend 接口实现
 * 参考 Common/Audio/OpenSLESAudioBackend.cpp
 */

#include "ohos_audio_backend.h"
#include "ohos_hilog.h"
#include "Common/Log.h"
#include "Common/Math/math_util.h"
#include <cstring>
#include <algorithm>

#define AUDIO_BACKEND_TAG "PPSSPP_AudioBackend"

// 全局实例指针（用于静态回调）
static OhosAudioBackend *g_audioBackend = nullptr;

OhosAudioBackend::OhosAudioBackend() {
    OHOS_LOGI(AUDIO_BACKEND_TAG, "OhosAudioBackend created");
    g_audioBackend = this;
}

OhosAudioBackend::~OhosAudioBackend() {
    if (audioState_) {
        OhosAudio_Pause(audioState_);
        OhosAudio_Shutdown(audioState_);
        audioState_ = nullptr;
    }
    
    if (mixBuffer_) {
        delete[] mixBuffer_;
        mixBuffer_ = nullptr;
    }
    
    g_audioBackend = nullptr;
    OHOS_LOGI(AUDIO_BACKEND_TAG, "OhosAudioBackend destroyed");
}

void OhosAudioBackend::EnumerateDevices(std::vector<AudioDeviceDesc> *outputDevices, bool captureDevices) {
    // OHOS 通常只有一个默认音频设备
    if (!captureDevices) {
        AudioDeviceDesc desc;
        desc.name = "Default Audio Device";
        desc.uniqueId = "default";
        outputDevices->push_back(desc);
    }
}

void OhosAudioBackend::SetRenderCallback(RenderCallback callback, void *userdata) {
    std::lock_guard<std::mutex> lock(mutex_);
    renderCallback_ = callback;
    renderUserdata_ = userdata;
    OHOS_LOGI(AUDIO_BACKEND_TAG, "SetRenderCallback: callback=%{public}p, userdata=%{public}p", callback, userdata);
}

bool OhosAudioBackend::InitOutputDevice(std::string_view uniqueId, LatencyMode latencyMode, bool *revertedToDefault) {
    OHOS_LOGI(AUDIO_BACKEND_TAG, "InitOutputDevice: uniqueId=%{public}s, latencyMode=%{public}d", 
             std::string(uniqueId).c_str(), (int)latencyMode);
    
    // 根据延迟模式选择缓冲区大小
    if (latencyMode == LatencyMode::Aggressive) {
        framesPerBuffer_ = 256;  // 低延迟
    } else {
        framesPerBuffer_ = 512;  // 安全模式
    }
    
    // 初始化音频状态
    audioState_ = OhosAudio_Init(AudioCallbackStatic, framesPerBuffer_, sampleRate_);
    if (!audioState_) {
        OHOS_LOGE(AUDIO_BACKEND_TAG, "Failed to create OhosAudioState");
        return false;
    }
    
    // 分配混音缓冲区（float）
    mixBufferSize_ = framesPerBuffer_ * 2;  // stereo
    mixBuffer_ = new float[mixBufferSize_];
    
    // 启动音频
    if (!OhosAudio_Resume(audioState_)) {
        OHOS_LOGE(AUDIO_BACKEND_TAG, "Failed to resume audio");
        OhosAudio_Shutdown(audioState_);
        audioState_ = nullptr;
        delete[] mixBuffer_;
        mixBuffer_ = nullptr;
        return false;
    }
    
    if (revertedToDefault) {
        *revertedToDefault = false;
    }
    
    OHOS_LOGI(AUDIO_BACKEND_TAG, "Audio output device initialized: rate=%{public}d, buffer=%{public}d", 
             sampleRate_, framesPerBuffer_);
    return true;
}

int OhosAudioBackend::SampleRate() const {
    return sampleRate_;
}

int OhosAudioBackend::BufferSize() const {
    return framesPerBuffer_;
}

int OhosAudioBackend::PeriodFrames() const {
    return framesPerBuffer_;
}

void OhosAudioBackend::DescribeOutputFormat(char *buffer, size_t bufferSize) const {
    snprintf(buffer, bufferSize, "OHAudio: %d Hz, %d frames", sampleRate_, framesPerBuffer_);
}

void OhosAudioBackend::FrameUpdate(bool allowAutoChange) {
    // 不需要特殊处理
}

void OhosAudioBackend::AudioCallbackStatic(short *buffer, int numSamples, int sampleRateHz) {
    if (g_audioBackend) {
        g_audioBackend->AudioCallback(buffer, numSamples, sampleRateHz);
    } else {
        // 没有后端，填充静音
        memset(buffer, 0, numSamples * sizeof(short));
    }
}

// 全局音频重置函数 - 由 System_Notify(AUDIO_RESET_DEVICE) 调用
void OhosAudio_ResetDevice() {
    OHOS_LOGI(AUDIO_BACKEND_TAG, "OhosAudio_ResetDevice called");
    
    if (g_audioBackend) {
        // 暂停并重新初始化音频
        // 注意：这里简单实现，实际可能需要更复杂的重置逻辑
        OHOS_LOGI(AUDIO_BACKEND_TAG, "Audio device reset requested (no-op for now)");
    }
}

void OhosAudioBackend::AudioCallback(short *buffer, int numSamples, int sampleRateHz) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 调试：每 100 次回调打印一次（更频繁）
    static int callCount = 0;
    callCount++;
    bool shouldLog = (callCount % 100 == 0);
    
//    if (shouldLog) {
//        OHOS_LOGI(AUDIO_BACKEND_TAG, "AudioCallback #%{public}d: renderCallback_=%{public}p, numSamples=%{public}d", 
//                 callCount, renderCallback_, numSamples);
//    }
    
    if (!renderCallback_) {
        // 没有回调，填充静音
        memset(buffer, 0, numSamples * sizeof(short));
        if (shouldLog) {
            OHOS_LOGW(AUDIO_BACKEND_TAG, "No render callback set! Outputting silence.");
        }
        return;
    }
    
    // numSamples 是样本数（stereo 时 = numFrames * 2）
    int numFrames = numSamples / 2;
    
    // 动态扩展混音缓冲区（如果需要）
    if (numSamples > mixBufferSize_) {
        OHOS_LOGI(AUDIO_BACKEND_TAG, "Expanding mix buffer: %{public}d -> %{public}d", 
                  mixBufferSize_, numSamples);
        delete[] mixBuffer_;
        mixBufferSize_ = numSamples;
        mixBuffer_ = new float[mixBufferSize_];
    }
    
    // 调用 PPSSPP 的渲染回调（输出 float）
    renderCallback_(mixBuffer_, numFrames, sampleRateHz, renderUserdata_);
    
    // 检查是否有非零数据
    float maxSample = 0.0f;
    for (int i = 0; i < numSamples; i++) {
        float abs_sample = fabs(mixBuffer_[i]);
        if (abs_sample > maxSample) {
            maxSample = abs_sample;
        }
    }
    
//    if (shouldLog) {
//        OHOS_LOGI(AUDIO_BACKEND_TAG, "Max sample value: %{public}.4f (numFrames=%{public}d, rate=%{public}d)", 
//                 maxSample, numFrames, sampleRateHz);
//    }
    
    // 如果 PPSSPP 没有音频数据，输出静音
    if (maxSample < 0.001f) {
        memset(buffer, 0, numSamples * sizeof(short));
//        if (shouldLog) {
//            OHOS_LOGD(AUDIO_BACKEND_TAG, "PPSSPP silent, outputting silence");
//        }
        return;
    }
    
    // 转换 float 到 short（-1.0~1.0 -> -32768~32767）
    for (int i = 0; i < numSamples; i++) {
        float sample = mixBuffer_[i];
        // 限幅
        sample = clamp_value(sample, -1.0f, 1.0f);
        // 转换到 16-bit
        buffer[i] = (short)(sample * 32767.0f);
    }
}
