# XComponent 使用指南

## 概述

本文档说明如何在 ArkTS 中使用 XComponent 集成 PPSSPP 渲染。

## 基本用法

### 1. 导入 Native 模块

```typescript
import ppsspp from 'libppsspp_ohos.so'
```

### 2. 创建 XComponent

```typescript
@Entry
@Component
struct GamePage {
  private xComponentController: XComponentController = new XComponentController()
  private surfaceId: string = ''
  
  build() {
    Column() {
      XComponent({
        id: 'ppsspp_surface',
        type: 'surface',
        controller: this.xComponentController
      })
      .onLoad((xComponentContext) => {
        // XComponent 加载完成
        this.surfaceId = xComponentContext.surfaceId
        console.info('XComponent loaded, surfaceId: ' + this.surfaceId)
        
        // 初始化 PPSSPP XComponent
        let result = ppsspp.initXComponent(this.xComponentController)
        if (result) {
          console.info('PPSSPP XComponent initialized successfully')
          
          // 初始化模拟器
          ppsspp.initEmulator()
        } else {
          console.error('Failed to initialize PPSSPP XComponent')
        }
      })
      .onDestroy(() => {
        // XComponent 销毁
        console.info('XComponent destroyed')
        ppsspp.shutdownEmulator()
      })
      .width('100%')
      .height('100%')
    }
  }
}
```

## 完整示例

### GameView.ets

```typescript
import ppsspp from 'libppsspp_ohos.so'

@Component
export struct GameView {
  private xComponentController: XComponentController = new XComponentController()
  @State isInitialized: boolean = false
  @State currentGame: string = ''
  
  aboutToAppear() {
    console.info('GameView aboutToAppear')
  }
  
  aboutToDisappear() {
    console.info('GameView aboutToDisappear')
    if (this.isInitialized) {
      ppsspp.shutdownEmulator()
      this.isInitialized = false
    }
  }
  
  private initPPSSPP() {
    try {
      // 1. 初始化 XComponent
      let result = ppsspp.initXComponent(this.xComponentController)
      if (!result) {
        console.error('Failed to initialize XComponent')
        return
      }
      
      // 2. 初始化模拟器
      result = ppsspp.initEmulator()
      if (!result) {
        console.error('Failed to initialize emulator')
        return
      }
      
      this.isInitialized = true
      console.info('PPSSPP initialized successfully')
    } catch (error) {
      console.error('Error initializing PPSSPP: ' + error)
    }
  }
  
  loadGame(gamePath: string) {
    if (!this.isInitialized) {
      console.error('PPSSPP not initialized')
      return
    }
    
    try {
      let result = ppsspp.loadGame(gamePath)
      if (result) {
        this.currentGame = gamePath
        console.info('Game loaded: ' + gamePath)
      } else {
        console.error('Failed to load game: ' + gamePath)
      }
    } catch (error) {
      console.error('Error loading game: ' + error)
    }
  }
  
  pauseGame() {
    if (this.isInitialized) {
      ppsspp.pauseEmulator()
    }
  }
  
  resumeGame() {
    if (this.isInitialized) {
      ppsspp.resumeEmulator()
    }
  }
  
  build() {
    Stack() {
      // 渲染表面
      XComponent({
        id: 'ppsspp_surface',
        type: 'surface',
        controller: this.xComponentController
      })
      .onLoad((xComponentContext) => {
        console.info('XComponent loaded')
        this.initPPSSPP()
      })
      .onDestroy(() => {
        console.info('XComponent destroyed')
      })
      .width('100%')
      .height('100%')
      
      // 控制按钮（可选）
      if (this.isInitialized) {
        Row() {
          Button('Pause')
            .onClick(() => this.pauseGame())
          
          Button('Resume')
            .onClick(() => this.resumeGame())
        }
        .position({ x: 0, y: 0 })
        .padding(10)
      }
    }
    .width('100%')
    .height('100%')
  }
}
```

### Index.ets (主页面)

```typescript
import { GameView } from './GameView'

@Entry
@Component
struct Index {
  @State selectedGame: string = ''
  
  build() {
    Column() {
      // 游戏选择界面
      if (this.selectedGame === '') {
        Column() {
          Text('PPSSPP for HarmonyOS')
            .fontSize(24)
            .margin(20)
          
          Button('Select Game')
            .onClick(() => {
              // TODO: 打开文件选择器
              // 这里暂时硬编码一个路径
              this.selectedGame = '/data/storage/el2/base/files/game.iso'
            })
        }
        .width('100%')
        .height('100%')
        .justifyContent(FlexAlign.Center)
      } else {
        // 游戏运行界面
        GameView()
      }
    }
    .width('100%')
    .height('100%')
  }
}
```

## 生命周期管理

### 初始化顺序

1. XComponent 创建
2. `onLoad` 回调触发
3. `initXComponent()` - 注册 XComponent 回调
4. `initEmulator()` - 初始化 PPSSPP
5. `loadGame()` - 加载游戏

### 清理顺序

1. `onDestroy` 回调触发
2. `shutdownEmulator()` - 关闭 PPSSPP
3. XComponent 销毁

## 触摸事件处理

```typescript
XComponent({
  id: 'ppsspp_surface',
  type: 'surface',
  controller: this.xComponentController
})
.onTouch((event: TouchEvent) => {
  // 触摸事件会自动传递到 Native 层
  // 也可以在这里手动处理
  if (event.type === TouchType.Down) {
    console.info('Touch down at: ' + event.touches[0].x + ', ' + event.touches[0].y)
  }
})
```

## 配置管理

```typescript
// 获取配置
let config = ppsspp.getConfig('Graphics.RenderingMode')
console.info('Current rendering mode: ' + config)

// 设置配置
ppsspp.setConfig('Graphics.RenderingMode', '1')  // 1 = Buffered
ppsspp.setConfig('Graphics.AnisotropyLevel', '4')
```

## 常见配置项

| 配置键 | 说明 | 值 |
|--------|------|-----|
| Graphics.RenderingMode | 渲染模式 | 0=Non-Buffered, 1=Buffered |
| Graphics.AnisotropyLevel | 各向异性过滤 | 0, 2, 4, 8, 16 |
| Graphics.FrameSkip | 跳帧 | 0-9 |
| Graphics.InternalResolution | 内部分辨率 | 1-10 (1=480x272) |
| Audio.Enable | 音频开关 | true/false |
| Control.ShowTouchControls | 显示触摸控制 | true/false |

## 错误处理

```typescript
try {
  let result = ppsspp.loadGame(gamePath)
  if (!result) {
    // 加载失败
    AlertDialog.show({
      title: 'Error',
      message: 'Failed to load game',
      confirm: {
        value: 'OK',
        action: () => {}
      }
    })
  }
} catch (error) {
  // 异常处理
  console.error('Exception: ' + error)
}
```

## 性能优化建议

1. **使用 surface 类型的 XComponent**
   - 提供最佳性能
   - 直接渲染到 Native 表面

2. **避免频繁重建 XComponent**
   - 使用状态管理控制显示/隐藏
   - 不要在每次渲染时创建新实例

3. **合理设置渲染分辨率**
   - 根据设备性能调整 InternalResolution
   - 平衡画质和性能

4. **启用帧跳过**
   - 在性能不足时自动跳帧
   - 保持游戏速度稳定

## 调试

### 启用日志

```typescript
// 在 module.json5 中配置
{
  "module": {
    "abilities": [
      {
        "name": "EntryAbility",
        "srcEntry": "./ets/entryability/EntryAbility.ts",
        "description": "$string:EntryAbility_desc",
        "icon": "$media:icon",
        "label": "$string:EntryAbility_label",
        "startWindowIcon": "$media:icon",
        "startWindowBackground": "$color:start_window_background",
        "exported": true,
        "skills": [
          {
            "entities": [
              "entity.system.home"
            ],
            "actions": [
              "action.system.home"
            ]
          }
        ]
      }
    ],
    "requestPermissions": [
      {
        "name": "ohos.permission.READ_MEDIA",
        "reason": "$string:permission_read_media"
      },
      {
        "name": "ohos.permission.WRITE_MEDIA",
        "reason": "$string:permission_write_media"
      }
    ]
  }
}
```

### 查看日志

```bash
# 查看 PPSSPP 日志
hdc shell hilog | grep PPSSPP

# 查看 OpenGL 日志
hdc shell hilog | grep PPSSPP_GL

# 查看 XComponent 日志
hdc shell hilog | grep PPSSPP_XComponent
```

## 注意事项

1. **XComponent 必须在主线程创建**
2. **不要在 aboutToDisappear 之后调用 PPSSPP 函数**
3. **确保在 XComponent 销毁前调用 shutdownEmulator()**
4. **游戏文件路径必须是应用有权限访问的路径**
5. **首次运行需要初始化配置文件**

## 参考

- [HarmonyOS XComponent 开发指南](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/napi-xcomponent-guidelines-0000001281201106-V3)
- [PPSSPP 配置文档](https://www.ppsspp.org/docs/reference/settings/)
