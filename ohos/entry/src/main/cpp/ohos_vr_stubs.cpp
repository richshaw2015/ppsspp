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

// HarmonyOS VR 功能桩实现
// PPSSPP 的 VR 功能在 HarmonyOS 上暂不支持

// VR 相关函数的空实现（C++ 函数，不是 C）

bool IsVREnabled() {
    return false;
}

int GetVRPassesCount() {
    return 1;  // 非 VR 模式只有一个渲染 pass
}

void PreVRFrameRender(int pass) {
    // 空实现
}

void PostVRFrameRender() {
    // 空实现
}

void PreprocessStepVR(void* step) {
    // 空实现
}

int GetVRFBOIndex() {
    return 0;  // 默认 FBO
}

void BindVRFramebuffer() {
    // 空实现
}

bool Is2DVRObject(float* matrix, bool ortho) {
    return false;
}

bool IsFlatVRScene() {
    return false;
}

bool IsImmersiveVRMode() {
    return false;
}

void UpdateVRProjection(float* leftEye, float* rightEye) {
    // 空实现
}

void UpdateVRParams(float* params) {
    // 空实现
}

enum VRCompatFlag {
    VR_COMPAT_NONE = 0,
};

void SetVRCompat(VRCompatFlag flag, long value) {
    // 空实现
}

void UpdateVRView(float* leftEye, float* rightEye) {
    // 空实现
}

bool IsBigScreenVRMode() {
    return false;
}

bool IsFlatVRGame() {
    return false;
}
