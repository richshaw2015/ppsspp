/**
 * PPSSPP Native 接口封装
 * 提供 ArkTS 调用 Native C++ 的接口
 */

import ppsspp from 'libppsspp_ohos.so';

export class PPSSPPNative {
  /**
   * 初始化模拟器
   */
  static initEmulator(): boolean {
    try {
      return ppsspp.initEmulator();
    } catch (error) {
      console.error('Failed to init emulator:', error);
      return false;
    }
  }

  /**
   * 关闭模拟器
   */
  static shutdownEmulator(): void {
    try {
      ppsspp.shutdownEmulator();
    } catch (error) {
      console.error('Failed to shutdown emulator:', error);
    }
  }

  /**
   * 加载游戏
   * @param gamePath 游戏文件路径
   */
  static loadGame(gamePath: string): boolean {
    try {
      return ppsspp.loadGame(gamePath);
    } catch (error) {
      console.error('Failed to load game:', error);
      return false;
    }
  }

  /**
   * 运行一帧
   */
  static runFrame(): void {
    try {
      ppsspp.runFrame();
    } catch (error) {
      console.error('Failed to run frame:', error);
    }
  }

  /**
   * 暂停模拟器
   */
  static pauseEmulator(): void {
    try {
      ppsspp.pauseEmulator();
    } catch (error) {
      console.error('Failed to pause emulator:', error);
    }
  }

  /**
   * 恢复模拟器
   */
  static resumeEmulator(): void {
    try {
      ppsspp.resumeEmulator();
    } catch (error) {
      console.error('Failed to resume emulator:', error);
    }
  }

  /**
   * 发送触摸事件
   * @param x X 坐标
   * @param y Y 坐标
   * @param action 动作类型 (0=down, 1=move, 2=up)
   */
  static sendTouchEvent(x: number, y: number, action: number): void {
    try {
      ppsspp.sendTouchEvent(x, y, action);
    } catch (error) {
      console.error('Failed to send touch event:', error);
    }
  }

  /**
   * 发送按键事件
   * @param keyCode 按键码
   * @param isDown 是否按下
   */
  static sendKeyEvent(keyCode: number, isDown: boolean): void {
    try {
      ppsspp.sendKeyEvent(keyCode, isDown);
    } catch (error) {
      console.error('Failed to send key event:', error);
    }
  }

  /**
   * 获取配置
   * @param key 配置键
   */
  static getConfig(key: string): string {
    try {
      return ppsspp.getConfig(key);
    } catch (error) {
      console.error('Failed to get config:', error);
      return '';
    }
  }

  /**
   * 设置配置
   * @param key 配置键
   * @param value 配置值
   */
  static setConfig(key: string, value: string): void {
    try {
      ppsspp.setConfig(key, value);
    } catch (error) {
      console.error('Failed to set config:', error);
    }
  }
}
