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
   * 注册 XComponent
   * 在 XComponent onLoad 时调用，确保 native 层可以获取到 XComponent
   * @param xcomponentId XComponent 的 ID
   * @returns 成功返回 true
   */
  function registerXComponent(xcomponentId: string): boolean;

  /**
   * 初始化系统属性
   * @param deviceName 设备名称
   * @param deviceBuild 设备构建信息
   * @param language 语言设置
   * @param boardName 主板名称
   * @param osVersion 操作系统版本
   * @param devType 设备类型 (0=手机, 1=平板, 2=电视)
   * @param xres 屏幕宽度
   * @param yres 屏幕高度
   * @param dpi 屏幕 DPI
   * @param refreshRate 屏幕刷新率
   * @returns 成功返回 true
   */
  function initSystemProperties(
    deviceName: string,
    deviceBuild: string,
    language: string,
    boardName: string,
    osVersion: number,
    devType: number,
    xres: number,
    yres: number,
    dpi: number,
    refreshRate: number
  ): boolean;

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
   * @param keyCode 按键码（OHOS KeyCode）
   * @param isDown 是否按下
   * @param isRepeat 是否是重复按键（可选）
   * @param deviceId 设备 ID（可选，0=默认，1=键盘，10-19=手柄）
   * @returns 是否消费了该按键
   */
  function sendKeyEvent(keyCode: number, isDown: boolean, isRepeat?: boolean, deviceId?: number): boolean;

  /**
   * 发送单个手柄轴事件
   * @param deviceId 设备 ID（0-9 对应手柄 0-9）
   * @param axisId 轴 ID（0=X, 1=Y, 11=Z, 12=RX, 13=RY, 14=RZ, 15=HAT_X, 16=HAT_Y, 17=LTRIGGER, 18=RTRIGGER）
   * @param value 轴值（-1.0 到 1.0）
   */
  function sendAxisEvent(deviceId: number, axisId: number, value: number): void;

  /**
   * 发送多个手柄轴事件（批量处理，更高效）
   * @param deviceId 设备 ID
   * @param axisIds 轴 ID 数组
   * @param values 轴值数组
   */
  function sendMultiAxisEvent(deviceId: number, axisIds: number[], values: number[]): void;

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

  /**
   * 设置打开 URL 的回调函数
   * 当 PPSSPP 需要打开外部链接时会调用此回调
   * @param callback 回调函数，参数为 URL 字符串，返回是否成功打开
   */
  function setOpenUrlCallback(callback: (url: string) => boolean): boolean;

  /**
   * 设置浏览图片的回调函数
   * 当 PPSSPP 需要选择图片（如自定义壁纸）时会调用此回调
   * @param callback 回调函数，参数为请求 ID
   */
  function setBrowseImageCallback(callback: (requestId: number) => void): boolean;

  /**
   * 图片选择完成后的回调
   * 从 ArkTS 层调用，通知 C++ 层选择结果
   * @param requestId 请求 ID
   * @param success 是否成功选择
   * @param path 选择的图片路径
   */
  function onImageSelected(requestId: number, success: boolean, path: string): boolean;

  /**
   * 设置浏览文件的回调函数
   * 当 PPSSPP 需要选择文件（如游戏 ISO、存档等）时会调用此回调
   * @param callback 回调函数，参数为请求 ID 和文件类型
   * 文件类型: 0=BOOTABLE, 1=IMAGE, 2=INI, 3=DB, 4=SOUND_EFFECT, 5=ZIP, 6=SYMBOL_MAP, 7=SYMBOL_MAP_NOCASH, 8=ATRAC3, 9=ANY
   */
  function setBrowseFileCallback(callback: (requestId: number, fileType: number) => void): boolean;

  /**
   * 文件选择完成后的回调
   * @param requestId 请求 ID
   * @param success 是否成功选择
   * @param path 选择的文件路径
   */
  function onFileSelected(requestId: number, success: boolean, path: string): boolean;

  /**
   * 设置浏览文件夹的回调函数
   * 当 PPSSPP 需要选择文件夹（如游戏目录）时会调用此回调
   * @param callback 回调函数，参数为请求 ID
   */
  function setBrowseFolderCallback(callback: (requestId: number) => void): boolean;

  /**
   * 文件夹选择完成后的回调
   * @param requestId 请求 ID
   * @param success 是否成功选择
   * @param path 选择的文件夹路径
   */
  function onFolderSelected(requestId: number, success: boolean, path: string): boolean;

  /**
   * 设置文本输入对话框的回调函数
   * 当 PPSSPP 需要用户输入文本（如重命名存档、输入金手指）时会调用此回调
   * @param callback 回调函数，参数为请求 ID、标题和默认文本
   */
  function setInputTextCallback(callback: (requestId: number, title: string, defaultText: string) => void): boolean;

  /**
   * 文本输入完成后的回调
   * @param requestId 请求 ID
   * @param success 是否成功输入（false 表示用户取消）
   * @param text 用户输入的文本
   */
  function onInputTextCompleted(requestId: number, success: boolean, text: string): boolean;

  /**
   * 设置 Toast 回调函数
   * 当 PPSSPP 需要显示 Toast 提示时会调用此回调
   * @param callback 回调函数，参数为消息文本
   */
  function setToastCallback(callback: (message: string) => void): boolean;

  /**
   * 设置剪贴板回调函数
   * 当 PPSSPP 需要复制文本到剪贴板时会调用此回调
   * @param callback 回调函数，参数为要复制的文本
   */
  function setClipboardCallback(callback: (text: string) => void): boolean;

  /**
   * 设置屏幕常亮回调函数
   * 当 PPSSPP 需要设置屏幕常亮状态时会调用此回调
   * @param callback 回调函数，参数为是否保持屏幕常亮
   */
  function setKeepScreenOnCallback(callback: (keepOn: boolean) => void): boolean;

  /**
   * 设置分享文本回调函数
   * 当 PPSSPP 需要分享文本时会调用此回调
   * @param callback 回调函数，参数为要分享的文本
   */
  function setShareTextCallback(callback: (text: string) => void): boolean;

  /**
   * 设置显示文件位置回调函数
   * 当 PPSSPP 需要在文件管理器中显示文件时会调用此回调
   * @param callback 回调函数，参数为文件路径
   */
  function setShowFileInFolderCallback(callback: (path: string) => void): boolean;

  /**
   * 设置重启应用回调函数
   * 当 PPSSPP 需要重启应用（如切换渲染引擎）时会调用此回调
   * @param callback 回调函数，参数为重启参数
   */
  function setRestartAppCallback(callback: (params: string) => void): boolean;

  /**
   * 设置退出应用回调函数
   * 当 PPSSPP 需要退出应用时会调用此回调
   * @param callback 回调函数
   */
  function setExitAppCallback(callback: () => void): boolean;

  /**
   * 设置重建 Activity 回调函数
   * 当 PPSSPP 需要重建 Activity（如显示设置变更）时会调用此回调
   * @param callback 回调函数
   */
  function setRecreateActivityCallback(callback: () => void): boolean;

  /**
   * 设置沉浸模式回调函数
   * 当 PPSSPP 需要设置沉浸模式时会调用此回调
   * @param callback 回调函数，参数为是否启用沉浸模式
   */
  function setImmersiveModeCallback(callback: (immersive: boolean) => void): boolean;

  /**
   * 设置屏幕旋转回调函数
   * 当 PPSSPP 需要更新屏幕旋转时会调用此回调
   * @param callback 回调函数
   */
  function setScreenRotationCallback(callback: () => void): boolean;

  /**
   * 设置显示软键盘回调函数
   * 当 PPSSPP 需要显示软键盘时会调用此回调
   * @param callback 回调函数
   */
  function setShowKeyboardCallback(callback: () => void): boolean;
}

export default ppsspp;
