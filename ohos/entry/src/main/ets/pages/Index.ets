import ppsspp from 'libppsspp_ohos.so';

import common from '@ohos.app.ability.common';
import windowManager from '@ohos.window';
import vibrator from '@ohos.vibrator';
import { Want } from '@kit.AbilityKit';
import { photoAccessHelper } from '@kit.MediaLibraryKit';
import { fileIo } from '@kit.CoreFileKit';
import { picker } from '@kit.CoreFileKit';

@Entry
@Component
struct Index {
  private xComponentId: string = 'ppsspp_xcomponent';
  private xComponentController: XComponentController = new XComponentController();
  @State isInitialized: boolean = false;
  private context: common.UIAbilityContext = getContext() as common.UIAbilityContext;
  private lastKeyCode: number = 0;  // 用于检测重复按键

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
   * 设置浏览图片回调函数
   * 当 PPSSPP 需要选择图片（如自定义壁纸）时会调用此回调
   */
  private setupBrowseImageCallback(): void {
    console.info('[PPSSPP] Setting up browse image callback');
    
    try {
      const success: boolean = ppsspp.setBrowseImageCallback(async (requestId: number): Promise<void> => {
        console.info(`[PPSSPP] Browse image requested, requestId: ${requestId}`);
        
        try {
          // 创建图片选择器
          const photoPicker = new photoAccessHelper.PhotoViewPicker();
          
          // 配置选择选项
          const photoSelectOptions = new photoAccessHelper.PhotoSelectOptions();
          photoSelectOptions.MIMEType = photoAccessHelper.PhotoViewMIMETypes.IMAGE_TYPE;
          photoSelectOptions.maxSelectNumber = 1;
          
          // 打开图片选择器
          const photoSelectResult = await photoPicker.select(photoSelectOptions);
          
          if (photoSelectResult && photoSelectResult.photoUris && photoSelectResult.photoUris.length > 0) {
            const selectedUri = photoSelectResult.photoUris[0];
            console.info(`[PPSSPP] Image selected: ${selectedUri}`);
            
            // 将图片复制到应用沙箱目录，因为 PPSSPP 需要直接访问文件
            const destPath = await this.copyImageToSandbox(selectedUri, requestId);
            
            if (destPath) {
              console.info(`[PPSSPP] Image copied to: ${destPath}`);
              ppsspp.onImageSelected(requestId, true, destPath);
            } else {
              console.error('[PPSSPP] Failed to copy image to sandbox');
              ppsspp.onImageSelected(requestId, false, '');
            }
          } else {
            console.info('[PPSSPP] No image selected (user cancelled)');
            ppsspp.onImageSelected(requestId, false, '');
          }
        } catch (err) {
          console.error(`[PPSSPP] Error selecting image: ${(err as Error).message}`);
          ppsspp.onImageSelected(requestId, false, '');
        }
      });
      
      if (success) {
        console.info('[PPSSPP] Browse image callback set successfully');
      } else {
        console.error('[PPSSPP] Failed to set browse image callback');
      }
    } catch (err) {
      console.error(`[PPSSPP] Error setting browse image callback: ${(err as Error).message}`);
    }
  }

  /**
   * 将选择的图片复制到应用沙箱目录
   * @param uri 图片 URI
   * @param requestId 请求 ID（用于生成唯一文件名）
   * @returns 复制后的文件路径，失败返回 null
   */
  private async copyImageToSandbox(uri: string, requestId: number): Promise<string | null> {
    try {
      // 打开源文件
      const srcFile = await fileIo.open(uri, fileIo.OpenMode.READ_ONLY);
      
      // 获取文件扩展名
      let ext = '.png';
      if (uri.toLowerCase().includes('.jpg') || uri.toLowerCase().includes('.jpeg')) {
        ext = '.jpg';
      } else if (uri.toLowerCase().includes('.png')) {
        ext = '.png';
      } else if (uri.toLowerCase().includes('.bmp')) {
        ext = '.bmp';
      }
      
      // 生成目标文件路径
      const timestamp = Date.now();
      const destFileName = `wallpaper_${requestId}_${timestamp}${ext}`;
      const destPath = `${this.context.filesDir}/${destFileName}`;
      
      // 创建目标文件
      const destFile = await fileIo.open(destPath, fileIo.OpenMode.CREATE | fileIo.OpenMode.WRITE_ONLY);
      
      // 复制文件内容
      const bufferSize = 4096;
      const buffer = new ArrayBuffer(bufferSize);
      let bytesRead: number;
      
      do {
        bytesRead = await fileIo.read(srcFile.fd, buffer);
        if (bytesRead > 0) {
          await fileIo.write(destFile.fd, buffer, { length: bytesRead });
        }
      } while (bytesRead > 0);
      
      // 关闭文件
      await fileIo.close(srcFile);
      await fileIo.close(destFile);
      
      console.info(`[PPSSPP] Image copied successfully: ${destPath}`);
      return destPath;
    } catch (err) {
      console.error(`[PPSSPP] Error copying image: ${(err as Error).message}`);
      return null;
    }
  }

  /**
   * 设置浏览文件回调函数
   * 当 PPSSPP 需要选择文件（如游戏 ISO、存档等）时会调用此回调
   */
  private setupBrowseFileCallback(): void {
    console.info('[PPSSPP] Setting up browse file callback');
    
    try {
      const success: boolean = ppsspp.setBrowseFileCallback(async (requestId: number, fileType: number): Promise<void> => {
        console.info(`[PPSSPP] Browse file requested, requestId: ${requestId}, fileType: ${fileType}`);
        
        try {
          // 根据文件类型确定文件后缀过滤
          let suffixFilters: string[] = [];
          switch (fileType) {
            case 0: // BOOTABLE - 游戏文件
              suffixFilters = ['.iso', '.cso', '.pbp', '.elf', '.prx', '.bin'];
              break;
            case 1: // IMAGE
              suffixFilters = ['.png', '.jpg', '.jpeg', '.bmp'];
              break;
            case 2: // INI
              suffixFilters = ['.ini'];
              break;
            case 3: // DB
              suffixFilters = ['.db'];
              break;
            case 4: // SOUND_EFFECT
              suffixFilters = ['.wav', '.mp3', '.ogg'];
              break;
            case 5: // ZIP
              suffixFilters = ['.zip'];
              break;
            case 6: // SYMBOL_MAP
            case 7: // SYMBOL_MAP_NOCASH
              suffixFilters = ['.map', '.sym'];
              break;
            case 8: // ATRAC3
              suffixFilters = ['.at3', '.aa3'];
              break;
            default: // ANY
              suffixFilters = ['*'];
              break;
          }
          
          // 创建文件选择器
          const documentPicker = new picker.DocumentViewPicker(this.context);
          const documentSelectOptions = new picker.DocumentSelectOptions();
          documentSelectOptions.maxSelectNumber = 1;
          // 注意：OHOS 的 DocumentViewPicker 可能不支持后缀过滤，这里仅作记录
          
          // 打开文件选择器
          const documentSelectResult = await documentPicker.select(documentSelectOptions);
          
          if (documentSelectResult && documentSelectResult.length > 0) {
            const selectedUri = documentSelectResult[0];
            console.info(`[PPSSPP] File selected: ${selectedUri}`);
            
            // 将文件复制到应用沙箱目录
            const destPath = await this.copyFileToSandbox(selectedUri, requestId);
            
            if (destPath) {
              console.info(`[PPSSPP] File copied to: ${destPath}`);
              ppsspp.onFileSelected(requestId, true, destPath);
            } else {
              console.error('[PPSSPP] Failed to copy file to sandbox');
              ppsspp.onFileSelected(requestId, false, '');
            }
          } else {
            console.info('[PPSSPP] No file selected (user cancelled)');
            ppsspp.onFileSelected(requestId, false, '');
          }
        } catch (err) {
          console.error(`[PPSSPP] Error selecting file: ${(err as Error).message}`);
          ppsspp.onFileSelected(requestId, false, '');
        }
      });
      
      if (success) {
        console.info('[PPSSPP] Browse file callback set successfully');
      } else {
        console.error('[PPSSPP] Failed to set browse file callback');
      }
    } catch (err) {
      console.error(`[PPSSPP] Error setting browse file callback: ${(err as Error).message}`);
    }
  }

  /**
   * 将选择的文件复制到应用沙箱目录
   * @param uri 文件 URI
   * @param requestId 请求 ID（用于生成唯一文件名）
   * @returns 复制后的文件路径，失败返回 null
   */
  private async copyFileToSandbox(uri: string, requestId: number): Promise<string | null> {
    try {
      // 打开源文件
      const srcFile = await fileIo.open(uri, fileIo.OpenMode.READ_ONLY);
      
      // 从 URI 中提取文件名
      let fileName = `file_${requestId}_${Date.now()}`;
      const uriParts = uri.split('/');
      if (uriParts.length > 0) {
        const lastPart = uriParts[uriParts.length - 1];
        if (lastPart && lastPart.includes('.')) {
          fileName = lastPart;
        }
      }
      
      // 生成目标文件路径
      const destPath = `${this.context.filesDir}/${fileName}`;
      
      // 创建目标文件
      const destFile = await fileIo.open(destPath, fileIo.OpenMode.CREATE | fileIo.OpenMode.WRITE_ONLY);
      
      // 复制文件内容
      const bufferSize = 65536; // 64KB
      const buffer = new ArrayBuffer(bufferSize);
      let bytesRead: number;
      
      do {
        bytesRead = await fileIo.read(srcFile.fd, buffer);
        if (bytesRead > 0) {
          await fileIo.write(destFile.fd, buffer, { length: bytesRead });
        }
      } while (bytesRead > 0);
      
      // 关闭文件
      await fileIo.close(srcFile);
      await fileIo.close(destFile);
      
      console.info(`[PPSSPP] File copied successfully: ${destPath}`);
      return destPath;
    } catch (err) {
      console.error(`[PPSSPP] Error copying file: ${(err as Error).message}`);
      return null;
    }
  }

  /**
   * 设置浏览文件夹回调函数
   * 当 PPSSPP 需要选择文件夹（如游戏目录）时会调用此回调
   */
  private setupBrowseFolderCallback(): void {
    console.info('[PPSSPP] Setting up browse folder callback');
    
    try {
      const success: boolean = ppsspp.setBrowseFolderCallback(async (requestId: number): Promise<void> => {
        console.info(`[PPSSPP] Browse folder requested, requestId: ${requestId}`);
        
        try {
          // 创建文件夹选择器
          const documentPicker = new picker.DocumentViewPicker(this.context);
          const documentSelectOptions = new picker.DocumentSelectOptions();
          documentSelectOptions.selectMode = picker.DocumentSelectMode.FOLDER;
          
          // 打开文件夹选择器
          const documentSelectResult = await documentPicker.select(documentSelectOptions);
          
          if (documentSelectResult && documentSelectResult.length > 0) {
            const selectedUri = documentSelectResult[0];
            console.info(`[PPSSPP] Folder selected: ${selectedUri}`);
            
            // 文件夹路径可以直接使用（如果是沙箱内的路径）
            // 或者需要获取持久化权限
            ppsspp.onFolderSelected(requestId, true, selectedUri);
          } else {
            console.info('[PPSSPP] No folder selected (user cancelled)');
            ppsspp.onFolderSelected(requestId, false, '');
          }
        } catch (err) {
          console.error(`[PPSSPP] Error selecting folder: ${(err as Error).message}`);
          ppsspp.onFolderSelected(requestId, false, '');
        }
      });
      
      if (success) {
        console.info('[PPSSPP] Browse folder callback set successfully');
      } else {
        console.error('[PPSSPP] Failed to set browse folder callback');
      }
    } catch (err) {
      console.error(`[PPSSPP] Error setting browse folder callback: ${(err as Error).message}`);
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
          this.setupBrowseImageCallback();
          this.setupBrowseFileCallback();
          this.setupBrowseFolderCallback();
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
          .onKeyEvent((event: KeyEvent): void => {
            // 处理按键事件（键盘、手柄等）
            const isDown: boolean = event.type === KeyType.Down;
            const isRepeat: boolean = event.type === KeyType.Down && event.keyCode === this.lastKeyCode;
            this.lastKeyCode = isDown ? event.keyCode : 0;
            
            // 发送按键事件到 PPSSPP
            // deviceId: 1 = 键盘
            const consumed: boolean = ppsspp.sendKeyEvent(event.keyCode, isDown, isRepeat, 1);
            
            // 如果 PPSSPP 消费了按键，阻止默认行为
            if (consumed) {
              event.stopPropagation();
            }
          })
          .focusable(true)
          .defaultFocus(true)
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
