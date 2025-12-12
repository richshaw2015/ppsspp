/**
 * 鸿蒙输入系统接口
 * 处理触摸、按键、手柄等输入
 * 
 * 参考 Android 实现：android/jni/app-android.cpp
 */

#ifndef OHOS_INPUT_H
#define OHOS_INPUT_H

#include <cstdint>

namespace OhosInput {

/**
 * 触摸动作类型
 */
enum class TouchAction {
    DOWN = 0,
    MOVE = 1,
    UP = 2,
    CANCEL = 3
};

/**
 * 初始化输入系统
 * 必须在使用其他输入函数之前调用
 */
bool Initialize();

/**
 * 关闭输入系统
 */
void Shutdown();

/**
 * 设置渲染器初始化状态
 * 只有渲染器初始化后才处理输入
 */
void SetRendererInited(bool inited);

/**
 * 检查渲染器是否已初始化
 */
bool IsRendererInited();

/**
 * 处理触摸事件
 * 注意：主要的触摸处理在 ohos_xcomponent.cpp 的 OnTouchEvent 中
 * 这个函数用于从 ArkTS 层直接发送触摸事件
 * 
 * @param x X 坐标（屏幕像素）
 * @param y Y 坐标（屏幕像素）
 * @param action 动作类型
 * @param pointerId 触摸点 ID（多点触控）
 */
void HandleTouchEvent(float x, float y, TouchAction action, int pointerId = 0);

/**
 * 处理按键事件
 * 从 ArkTS 层接收按键事件并转发到 PPSSPP
 * 
 * @param keyCode OHOS 按键码
 * @param isDown 是否按下（true=按下，false=抬起）
 * @param isRepeat 是否是重复按键
 * @param deviceId 设备 ID（0=默认，1=键盘，10-19=手柄）
 * @return 是否消费了该按键
 */
bool HandleKeyEvent(int keyCode, bool isDown, bool isRepeat = false, int deviceId = 0);

/**
 * 处理单个手柄轴事件
 * 
 * @param deviceId 设备 ID
 * @param axisId 轴 ID（0=X, 1=Y, 11=Z, 12=RX, 13=RY, 14=RZ, 15=HAT_X, 16=HAT_Y, 17=LTRIGGER, 18=RTRIGGER）
 * @param value 轴值（-1.0 到 1.0）
 */
void HandleAxisEvent(int deviceId, int axisId, float value);

/**
 * 处理多个手柄轴事件（批量处理，更高效）
 * 
 * @param deviceId 设备 ID
 * @param axisIds 轴 ID 数组
 * @param values 轴值数组
 * @param count 轴数量
 */
void HandleMultiAxisEvent(int deviceId, const int* axisIds, const float* values, int count);

} // namespace OhosInput

#endif // OHOS_INPUT_H
