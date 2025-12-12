/**
 * PPSSPP NAPI 接口声明
 */

#ifndef NAPI_PPSSPP_H
#define NAPI_PPSSPP_H

#include <napi/native_api.h>
#include <string>

namespace NapiPPSSPP {

/**
 * 初始化模拟器
 */
napi_value InitEmulator(napi_env env, napi_callback_info info);

/**
 * 关闭模拟器
 */
napi_value ShutdownEmulator(napi_env env, napi_callback_info info);

/**
 * 加载游戏
 */
napi_value LoadGame(napi_env env, napi_callback_info info);

/**
 * 运行一帧
 */
napi_value RunFrame(napi_env env, napi_callback_info info);

/**
 * 暂停模拟器
 */
napi_value PauseEmulator(napi_env env, napi_callback_info info);

/**
 * 恢复模拟器
 */
napi_value ResumeEmulator(napi_env env, napi_callback_info info);

/**
 * 发送触摸事件
 */
napi_value SendTouchEvent(napi_env env, napi_callback_info info);

/**
 * 发送按键事件
 * 参数: keyCode (number), isDown (boolean), isRepeat? (boolean), deviceId? (number)
 * 返回: 是否消费了该按键
 */
napi_value SendKeyEvent(napi_env env, napi_callback_info info);

/**
 * 发送单个手柄轴事件
 * 参数: deviceId (number), axisId (number), value (number)
 */
napi_value SendAxisEvent(napi_env env, napi_callback_info info);

/**
 * 发送多个手柄轴事件（批量处理，更高效）
 * 参数: deviceId (number), axisIds (number[]), values (number[])
 */
napi_value SendMultiAxisEvent(napi_env env, napi_callback_info info);

/**
 * 获取配置
 */
napi_value GetConfig(napi_env env, napi_callback_info info);

/**
 * 设置配置
 */
napi_value SetConfig(napi_env env, napi_callback_info info);

/**
 * 震动功能
 */
napi_value Vibrate(napi_env env, napi_callback_info info);

/**
 * 设置震动回调函数
 */
napi_value SetVibrationCallback(napi_env env, napi_callback_info info);

/**
 * 检查和设置震动配置
 */
napi_value CheckVibrationConfig(napi_env env, napi_callback_info info);

/**
 * 设置打开 URL 的回调函数
 * 参数: callback (function(url: string): boolean)
 */
napi_value SetOpenUrlCallback(napi_env env, napi_callback_info info);

/**
 * 从 C++ 层调用打开 URL
 * 内部使用，不导出到 ArkTS
 */
bool OpenUrl(const std::string& url);

/**
 * 设置浏览图片的回调函数
 * 参数: callback (function(requestId: number): void)
 */
napi_value SetBrowseImageCallback(napi_env env, napi_callback_info info);

/**
 * 从 C++ 层调用浏览图片
 * 内部使用，不导出到 ArkTS
 * @param requestId 请求 ID，用于回调时匹配
 */
bool BrowseForImage(int requestId);

/**
 * 图片选择完成后的回调
 * 从 ArkTS 层调用，通知 C++ 层选择结果
 * 参数: requestId (number), success (boolean), path (string)
 */
napi_value OnImageSelected(napi_env env, napi_callback_info info);

/**
 * 设置浏览文件的回调函数
 * 参数: callback (function(requestId: number, fileType: number): void)
 */
napi_value SetBrowseFileCallback(napi_env env, napi_callback_info info);

/**
 * 从 C++ 层调用浏览文件
 * @param requestId 请求 ID
 * @param fileType 文件类型 (BrowseFileType)
 */
bool BrowseForFile(int requestId, int fileType);

/**
 * 文件选择完成后的回调
 * 参数: requestId (number), success (boolean), path (string)
 */
napi_value OnFileSelected(napi_env env, napi_callback_info info);

/**
 * 设置浏览文件夹的回调函数
 * 参数: callback (function(requestId: number): void)
 */
napi_value SetBrowseFolderCallback(napi_env env, napi_callback_info info);

/**
 * 从 C++ 层调用浏览文件夹
 * @param requestId 请求 ID
 */
bool BrowseForFolder(int requestId);

/**
 * 文件夹选择完成后的回调
 * 参数: requestId (number), success (boolean), path (string)
 */
napi_value OnFolderSelected(napi_env env, napi_callback_info info);

/**
 * 设置文本输入对话框的回调函数
 * 参数: callback (function(requestId: number, title: string, defaultText: string): void)
 */
napi_value SetInputTextCallback(napi_env env, napi_callback_info info);

/**
 * 从 C++ 层调用显示文本输入对话框
 * @param requestId 请求 ID
 * @param title 对话框标题
 * @param defaultText 默认文本
 */
bool ShowInputTextDialog(int requestId, const std::string& title, const std::string& defaultText);

/**
 * 文本输入完成后的回调
 * 参数: requestId (number), success (boolean), text (string)
 */
napi_value OnInputTextCompleted(napi_env env, napi_callback_info info);

/**
 * 设置 Toast 回调函数
 * 参数: callback (function(message: string): void)
 */
napi_value SetToastCallback(napi_env env, napi_callback_info info);

/**
 * 从 C++ 层调用显示 Toast
 * @param message Toast 消息
 */
bool ShowToast(const std::string& message);

/**
 * 设置剪贴板回调函数
 * 参数: callback (function(text: string): void)
 */
napi_value SetClipboardCallback(napi_env env, napi_callback_info info);

/**
 * 从 C++ 层调用复制到剪贴板
 * @param text 要复制的文本
 */
bool CopyToClipboard(const std::string& text);

/**
 * 设置屏幕常亮回调函数
 * 参数: callback (function(keepOn: boolean): void)
 */
napi_value SetKeepScreenOnCallback(napi_env env, napi_callback_info info);

/**
 * 从 C++ 层调用设置屏幕常亮
 * @param keepOn 是否保持屏幕常亮
 */
bool SetKeepScreenOn(bool keepOn);

} // namespace NapiPPSSPP

#endif // NAPI_PPSSPP_H
