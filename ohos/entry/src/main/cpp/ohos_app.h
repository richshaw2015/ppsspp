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
