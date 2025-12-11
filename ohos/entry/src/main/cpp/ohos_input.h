/**
 * 鸿蒙输入系统接口
 * 处理触摸、按键、手柄等输入
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
 * 处理触摸事件
 * @param x X 坐标
 * @param y Y 坐标
 * @param action 动作类型
 * @param pointerId 触摸点 ID
 */
void HandleTouchEvent(float x, float y, TouchAction action, int pointerId = 0);

/**
 * 处理按键事件
 * @param keyCode 按键码
 * @param isDown 是否按下
 */
void HandleKeyEvent(int keyCode, bool isDown);

/**
 * 处理手柄事件
 * @param axis 轴类型
 * @param value 轴值 (-1.0 到 1.0)
 */
void HandleAxisEvent(int axis, float value);

/**
 * 初始化输入系统
 */
bool Initialize();

/**
 * 关闭输入系统
 */
void Shutdown();

} // namespace OhosInput

#endif // OHOS_INPUT_H
