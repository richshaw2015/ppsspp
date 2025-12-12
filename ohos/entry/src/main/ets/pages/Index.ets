import ppsspp from 'libppsspp_ohos.so';

import common from '@ohos.app.ability.common';
import windowManager from '@ohos.window';
import vibrator from '@ohos.vibrator';
import { Want } from '@kit.AbilityKit';

@Entry
@Component
struct Index {
  private xComponentId: string = 'ppsspp_xcomponent';
  private xComponentController: XComponentController = new XComponentController();
  @State isInitialized: boolean = false;
  private context: common.UIAbilityContext = getContext() as common.UIAbilityContext;

  /**
   * 检查震动权限和设备支持
   */
  private async checkVibrationPermission(): Promise<void> {
    console.info('[PPSSPP] Vibration permission should be granted via module.json5');
    
    // 检查设备是否支持震动
    try {
      const isSupportEffect = vibrator.isSupportEffect('haptic.clock.timer');
      console.info(`[PPSSPP] Device supports vibration effects: ${isSupportEffect}`);
    } catch (err) {
      console.error(`[PPSSPP] Failed to check vibration support: ${(err as Error).message}`);
    }
  }

  /**
   * 设置打开 URL 回调函数
   * 当 PPSSPP 需要打开外部链接（如"如何获得游戏？"）时会调用此回调
   */
  private setupOpenUrlCallback(): void {
    console.info('[PPSSPP] Setting up open URL callback');
    
    try {
      const success: boolean = ppsspp.setOpenUrlCallback((url: string): boolean => {
        console.info(`[PPSSPP] Opening URL: ${url}`);
        
        try {
          // 使用 Want 打开外部浏览器
          const want: Want = {
            action: 'ohos.want.action.viewData',
            uri: url
          };
          
          this.context.startAbility(want).then(() => {
            console.info('[PPSSPP] Successfully opened URL in browser');
          }).catch((err: Error) => {
            console.error(`[PPSSPP] Failed to open URL: ${err.message}`);
          });
          
          return true;
        } catch (err) {
          console.error(`[PPSSPP] Error opening URL: ${(err as Error).message}`);
          return false;
        }
      });
      
      if (success) {
        console.info('[PPSSPP] Open URL callback set successfully');
      } else {
        console.error('[PPSSPP] Failed to set open URL callback');
      }
    } catch (err) {
      console.error(`[PPSSPP] Error setting open URL callback: ${(err as Error).message}`);
    }
  }

  /**
   * 设置震动回调函数
   */
  private setupVibrationCallback(): void {
    console.info('[PPSSPP] Setting up vibration callback');
    
    try {
      const success: boolean = ppsspp.setVibrationCallback((duration: number): boolean => {
        // 限制震动时长范围（10-500ms，适合按键反馈）
        const clampedDuration = Math.max(10, Math.min(duration, 500));
        
        // 使用鸿蒙震动 API（使用 touch 用途，适合按键反馈）
        vibrator.startVibration({
          type: 'time',
          duration: clampedDuration
        }, {
          usage: 'touch'
        }).catch((err: Error) => {
          console.error(`[PPSSPP] Vibration failed: ${err.message}`);
        });
        
        return true;
      });
      
      if (success) {
        console.info('[PPSSPP] Vibration callback set successfully');
      } else {
        console.error('[PPSSPP] Failed to set vibration callback');
      }
    } catch (err) {
      console.error(`[PPSSPP] Error setting vibration callback: ${(err as Error).message}`);
    }
  }

  async aboutToAppear(): Promise<void> {
    console.info('[PPSSPP] Page aboutToAppear');
    
    // 初始化 PPSSPP 核心
    try {
      console.info('[PPSSPP] filesDir: ' + this.context.filesDir);
      console.info('[PPSSPP] cacheDir: ' + this.context.cacheDir);
      const success: boolean = ppsspp.initEmulator(
        this.context.filesDir,
        this.context.cacheDir,
        this.context.resourceManager  // 传递 ResourceManager 以直接读取 rawfile
      );
      
      if (success) {
				// 圆角屏幕的最小安全边距（像素）
				const MIN_CORNER_INSET = 40;

        // 获取并设置安全区域
        try {
          const windowClass = await windowManager.getLastWindow(this.context);
          
          // 获取系统避让区域（状态栏、导航栏）和刘海屏区域
          const systemAvoidArea = windowClass.getWindowAvoidArea(windowManager.AvoidAreaType.TYPE_SYSTEM);
          const cutoutAvoidArea = windowClass.getWindowAvoidArea(windowManager.AvoidAreaType.TYPE_CUTOUT);
          
          // 打印避让区域信息（用于调试）
          console.info(`[PPSSPP] systemAvoidArea - left: ${systemAvoidArea.leftRect.width}X${systemAvoidArea.leftRect.height}, right: ${systemAvoidArea.rightRect?.width}X${systemAvoidArea.rightRect?.height}`);
          console.info(`[PPSSPP] cutoutAvoidArea - left: ${cutoutAvoidArea.leftRect.width}X${cutoutAvoidArea.leftRect.height}, right: ${cutoutAvoidArea.rightRect.width}X${cutoutAvoidArea.rightRect.height}`);
          
          // 横屏游戏的安全区域策略：
          // 1. 使用 TYPE_CUTOUT（刘海屏）作为基础
          // 2. 左右对称：如果一侧有刘海，另一侧也使用相同宽度（视觉平衡）
          // 3. 为圆角屏幕添加最小安全边距（鸿蒙没有圆角 API）
          // 4. 不使用 TYPE_SYSTEM（状态栏/导航栏），因为横屏游戏会隐藏它们
          
          // 获取刘海区域
          const cutoutLeft = cutoutAvoidArea.leftRect?.width || 0;
          const cutoutTop = cutoutAvoidArea.topRect?.height || 0;
          const cutoutRight = cutoutAvoidArea.rightRect?.width || 0;
          const cutoutBottom = cutoutAvoidArea.bottomRect?.height || 0;
          
          // 横屏模式：左右对称（取左右两侧的最大值）
          const horizontalInset = Math.max(cutoutLeft, cutoutRight, MIN_CORNER_INSET);
          const verticalInset = Math.max(cutoutTop, cutoutBottom, MIN_CORNER_INSET);
          
          const leftInset = horizontalInset;
          const topInset = verticalInset;
          const rightInset = horizontalInset;
          const bottomInset = verticalInset;
          
          console.info(`[PPSSPP] Cutout insets: left=${cutoutLeft}, top=${cutoutTop}, right=${cutoutRight}, bottom=${cutoutBottom}`);
          console.info(`[PPSSPP] Final safe insets (with corner padding): left=${leftInset}, top=${topInset}, right=${rightInset}, bottom=${bottomInset}`);
          
          // 设置安全区域
          ppsspp.setSafeInsets(leftInset, topInset, rightInset, bottomInset);
        } catch (err) {
          console.error('[PPSSPP] Failed to get safe insets:', (err as Error).message);
          ppsspp.setSafeInsets(MIN_CORNER_INSET, MIN_CORNER_INSET, MIN_CORNER_INSET, MIN_CORNER_INSET);
        }
        
        // 检查震动权限
        this.checkVibrationPermission();
        
        // 延迟设置回调，确保 PPSSPP 核心已完全初始化
        setTimeout(() => {
          this.setupVibrationCallback();
          this.setupOpenUrlCallback();
        }, 500);
        
        this.isInitialized = true;
      } else {
        console.error('[PPSSPP] Failed to initialize emulator');
      }
    } catch (err) {
      const error = err as Error;
      console.error('[PPSSPP] Error:', error.message);
    }
  }

  aboutToDisappear() {
    console.info('[PPSSPP] Page aboutToDisappear');
    try {
      ppsspp.shutdownEmulator();
    } catch (err) {
      console.error('[PPSSPP] Shutdown error');
    }
  }

  build() {
    Stack() {
      // XComponent 用于渲染
      if (this.isInitialized) {
        XComponent({
          id: this.xComponentId,
          type: XComponentType.SURFACE,
          libraryname: 'ppsspp_ohos',
          controller: this.xComponentController
        })
          .onLoad(() => {
            console.info('[PPSSPP] XComponent onLoad');
            console.info('[PPSSPP] XComponent surfaceId: ' + this.xComponentController.getXComponentSurfaceId());
          })
          .onDestroy(() => {
            console.info('[PPSSPP] XComponent onDestroy');
          })
          .width('100%')
          .height('100%')
      }
      
    }
		// 横屏模式下扩展到安全区域
    .width('100%')
    .height('100%')
		.backgroundColor(Color.Black)
  }
}
