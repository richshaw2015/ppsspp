/**
 * PPSSPP Native 模块类型声明
 * 模块名: libppsspp_ohos.so
 */

declare namespace ppsspp {
  /**
   * 初始化 PPSSPP 模拟器
   * 调用 NativeInit 初始化核心系统
   * @param filesDir 应用文件目录 (context.filesDir)
   * @param cacheDir 应用缓存目录 (context.cacheDir)
   * @returns 成功返回 true
   */
  function initEmulator(filesDir: string, cacheDir: string): boolean;

  /**
   * 关闭 PPSSPP 模拟器
   */
  function shutdownEmulator(): void;

  /**
   * 加载游戏
   * @param gamePath 游戏文件路径
   * @returns 成功返回 true
   */
  function loadGame(gamePath: string): boolean;

  /**
   * 设置安全区域 insets（避开刘海屏等）
   * @param left 左侧安全区域（像素）
   * @param top 顶部安全区域（像素）
   * @param right 右侧安全区域（像素）
   * @param bottom 底部安全区域（像素）
   */
  function setSafeInsets(left: number, top: number, right: number, bottom: number): void;

  /**
   * 发送触摸事件
   * @param x X 坐标
   * @param y Y 坐标
   * @param action 动作类型 (0=按下, 1=移动, 2=抬起)
   */
  function sendTouchEvent(x: number, y: number, action: number): void;

  /**
   * 发送按键事件
   * @param keyCode 按键码
   * @param isDown 是否按下
   */
  function sendKeyEvent(keyCode: number, isDown: boolean): void;

  /**
   * 触发震动
   * @param duration 震动时长（毫秒）
   * @returns 是否成功触发震动
   */
  function vibrate(duration: number): boolean;

  /**
   * 设置震动回调函数
   * @param callback 震动回调函数，参数为震动时长（毫秒），返回是否成功
   */
  function setVibrationCallback(callback: (duration: number) => boolean): boolean;

  /**
   * 检查和设置震动配置
   * @returns 震动配置是否启用
   */
  function checkVibrationConfig(): boolean;
}

export default ppsspp;
