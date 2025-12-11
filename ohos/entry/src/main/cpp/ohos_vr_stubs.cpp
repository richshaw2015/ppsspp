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
