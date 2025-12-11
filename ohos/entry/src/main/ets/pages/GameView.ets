/**
 * 游戏渲染视图
 * 使用 XComponent 进行 Native 渲染
 */

import { PPSSPPNative } from '../napi/PPSSPPNative';

@Entry
@Component
struct GameView {
  private xComponentId: string = 'ppsspp_surface';
  private renderInterval: number = -1;

  aboutToAppear() {
    console.info('GameView aboutToAppear');
    // 初始化模拟器
    PPSSPPNative.initEmulator();
  }

  aboutToDisappear() {
    console.info('GameView aboutToDisappear');
    // 停止渲染循环
    if (this.renderInterval !== -1) {
      clearInterval(this.renderInterval);
      this.renderInterval = -1;
    }
    // 关闭模拟器
    PPSSPPNative.shutdownEmulator();
  }

  /**
   * 开始渲染循环
   */
  startRenderLoop() {
    if (this.renderInterval !== -1) {
      return;
    }

    // 60 FPS
    this.renderInterval = setInterval(() => {
      PPSSPPNative.runFrame();
    }, 16);
  }

  /**
   * 停止渲染循环
   */
  stopRenderLoop() {
    if (this.renderInterval !== -1) {
      clearInterval(this.renderInterval);
      this.renderInterval = -1;
    }
  }

  /**
   * 处理触摸事件
   */
  handleTouch(event: TouchEvent) {
    const touch = event.touches[0];
    let action = 0; // DOWN

    switch (event.type) {
      case TouchType.Down:
        action = 0;
        break;
      case TouchType.Move:
        action = 1;
        break;
      case TouchType.Up:
        action = 2;
        break;
    }

    PPSSPPNative.sendTouchEvent(touch.x, touch.y, action);
  }

  build() {
    Column() {
      // XComponent 用于 Native 渲染
      XComponent({
        id: this.xComponentId,
        type: 'surface',
        libraryname: 'ppsspp_ohos'
      })
        .onLoad(() => {
          console.info('XComponent onLoad');
          // 表面创建完成，可以开始渲染
          this.startRenderLoop();
        })
        .onDestroy(() => {
          console.info('XComponent onDestroy');
          this.stopRenderLoop();
        })
        .width('100%')
        .height('100%')
        .onTouch((event: TouchEvent) => {
          this.handleTouch(event);
        })

      // 控制按钮（示例）
      Row() {
        Button('加载游戏')
          .onClick(() => {
            // TODO: 打开文件选择器
            const gamePath = '/path/to/game.iso';
            PPSSPPNative.loadGame(gamePath);
          })
          .margin(10)

        Button('暂停')
          .onClick(() => {
            PPSSPPNative.pauseEmulator();
            this.stopRenderLoop();
          })
          .margin(10)

        Button('继续')
          .onClick(() => {
            PPSSPPNative.resumeEmulator();
            this.startRenderLoop();
          })
          .margin(10)
      }
      .width('100%')
      .justifyContent(FlexAlign.Center)
      .position({ x: 0, y: '90%' })
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#000000')
  }
}
