import { AbilityConstant, ConfigurationConstant, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { window, display } from '@kit.ArkUI';
import { BusinessError, deviceInfo } from '@kit.BasicServicesKit';
import { ppsspp } from 'libppsspp.so';

const DOMAIN = 0x0000;
const TAG = 'PPSSPP_EntryAbility';

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    try {
      this.context.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET);
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'Failed to set colorMode. Cause: %{public}s', JSON.stringify(err));
    }
    hilog.info(DOMAIN, TAG, '%{public}s', 'Ability onCreate');
    
    // 初始化系统属性（参考 Android 实现）
    this.initSystemProperties();
  }
  
  /**
   * 初始化系统属性
   * 从 HarmonyOS 系统获取设备信息并传递给 C++ 层
   */
  private initSystemProperties(): void {
    try {
      // 1. 获取设备信息
      const deviceName: string = deviceInfo.productModel || "HarmonyOS Device";
      const deviceBuild: string = `HarmonyOS ${deviceInfo.osFullName || "NEXT"}`;
      const osVersion: number = parseInt(deviceInfo.sdkApiVersion.toString()) || 5;
      
      hilog.info(DOMAIN, TAG, 'Device: %{public}s', deviceName);
      hilog.info(DOMAIN, TAG, 'Build: %{public}s', deviceBuild);
      hilog.info(DOMAIN, TAG, 'OS Version: %{public}d', osVersion);
      
      // 2. 获取语言设置
      const systemLanguage: string = this.context.config.language || "zh";
      // HarmonyOS 没有直接的 region API，使用语言代码推断
      const systemRegion: string = systemLanguage === "zh" ? "CN" : "US";
      const langRegion: string = `${systemLanguage}_${systemRegion}`;
      
      hilog.info(DOMAIN, TAG, 'Language: %{public}s', langRegion);
      
      // 3. 获取显示信息
      const displayClass = display.getDefaultDisplaySync();
      const xres: number = displayClass.width;
      const yres: number = displayClass.height;
      const dpi: number = displayClass.densityDPI;
      const refreshRate: number = displayClass.refreshRate || 60;
      
      hilog.info(DOMAIN, TAG, 'Display: %{public}dx%{public}d @%{public}dDPI %.1fHz', xres, yres, dpi, refreshRate);
      
      // 4. 设备类型判断
      let devType: number = 0;  // 0=手机
      if (deviceInfo.deviceType === 'tablet') {
        devType = 1;  // 平板
      } else if (deviceInfo.deviceType === 'tv') {
        devType = 2;  // 电视
      }
      
      hilog.info(DOMAIN, TAG, 'Device Type: %{public}d', devType);
      
      // 5. 调用 C++ 初始化函数
      ppsspp.initSystemProperties(
        deviceName,
        deviceBuild,
        langRegion,
        osVersion,
        devType,
        xres,
        yres,
        dpi,
        refreshRate
      );
      
      hilog.info(DOMAIN, TAG, 'System properties initialized successfully');
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'Failed to initialize system properties: %{public}s', JSON.stringify(err));
    }
  }

  onDestroy(): void {
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onDestroy');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    // Main window is created, set main page for this ability
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onWindowStageCreate');

		windowStage.getMainWindow().then((win) => {
			win.setWindowLayoutFullScreen(true);
			win.setWindowSystemBarEnable([]);
		});

    windowStage.loadContent('pages/Index', (err) => {
      if (err.code) {
        hilog.error(DOMAIN, 'testTag', 'Failed to load the content. Cause: %{public}s', JSON.stringify(err));
        return;
      }
      hilog.info(DOMAIN, 'testTag', 'Succeeded in loading the content.');
    });
  }

  onWindowStageDestroy(): void {
    // Main window is destroyed, release UI related resources
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onWindowStageDestroy');
  }

  onForeground(): void {
    // Ability has brought to foreground
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onForeground');
  }

  onBackground(): void {
    // Ability has back to background
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onBackground');
  }
}
