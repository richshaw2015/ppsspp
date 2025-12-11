# PPSSPP UI 库源文件列表
# 参考 android/jni/Android.mk 中的 ppsspp_jni 模块

set(UI_SOURCES
    # 核心应用
    ${PPSSPP_ROOT}/UI/NativeApp.cpp
    
    # 主要屏幕
    ${PPSSPP_ROOT}/UI/EmuScreen.cpp
    ${PPSSPP_ROOT}/UI/MainScreen.cpp
    ${PPSSPP_ROOT}/UI/GameScreen.cpp
    ${PPSSPP_ROOT}/UI/PauseScreen.cpp
    
    # 设置屏幕
    ${PPSSPP_ROOT}/UI/GameSettingsScreen.cpp
    ${PPSSPP_ROOT}/UI/ControlMappingScreen.cpp
    ${PPSSPP_ROOT}/UI/CustomButtonMappingScreen.cpp
    ${PPSSPP_ROOT}/UI/TouchControlLayoutScreen.cpp
    ${PPSSPP_ROOT}/UI/TouchControlVisibilityScreen.cpp
    ${PPSSPP_ROOT}/UI/TiltAnalogSettingsScreen.cpp
    ${PPSSPP_ROOT}/UI/DisplayLayoutScreen.cpp
    ${PPSSPP_ROOT}/UI/SystemInfoScreen.cpp
    ${PPSSPP_ROOT}/UI/DeveloperToolsScreen.cpp
    
    # 对话框和工具屏幕
    ${PPSSPP_ROOT}/UI/TabbedDialogScreen.cpp
    ${PPSSPP_ROOT}/UI/SimpleDialogScreen.cpp
    ${PPSSPP_ROOT}/UI/MiscScreens.cpp
    ${PPSSPP_ROOT}/UI/DevScreens.cpp
    ${PPSSPP_ROOT}/UI/SavedataScreen.cpp
    ${PPSSPP_ROOT}/UI/MemStickScreen.cpp
    ${PPSSPP_ROOT}/UI/InstallZipScreen.cpp
    ${PPSSPP_ROOT}/UI/CwCheatScreen.cpp
    ${PPSSPP_ROOT}/UI/ReportScreen.cpp
    ${PPSSPP_ROOT}/UI/UploadScreen.cpp
    
    # 商店和在线功能
    ${PPSSPP_ROOT}/UI/Store.cpp
    ${PPSSPP_ROOT}/UI/RemoteISOScreen.cpp
    ${PPSSPP_ROOT}/UI/ChatScreen.cpp
    ${PPSSPP_ROOT}/UI/RetroAchievementScreens.cpp
    
    # 驱动和测试
    ${PPSSPP_ROOT}/UI/DriverManagerScreen.cpp
    ${PPSSPP_ROOT}/UI/GPUDriverTestScreen.cpp
    ${PPSSPP_ROOT}/UI/JitCompareScreen.cpp
    
    # UI 组件和工具
    ${PPSSPP_ROOT}/UI/BaseScreens.cpp
    ${PPSSPP_ROOT}/UI/MiscViews.cpp
    ${PPSSPP_ROOT}/UI/Background.cpp
    ${PPSSPP_ROOT}/UI/Theme.cpp
    ${PPSSPP_ROOT}/UI/UIAtlas.cpp
    ${PPSSPP_ROOT}/UI/OnScreenDisplay.cpp
    ${PPSSPP_ROOT}/UI/DebugOverlay.cpp
    ${PPSSPP_ROOT}/UI/ProfilerDraw.cpp
    
    # 游戏相关
    ${PPSSPP_ROOT}/UI/GameInfoCache.cpp
    ${PPSSPP_ROOT}/UI/GamepadEmu.cpp
    ${PPSSPP_ROOT}/UI/JoystickHistoryView.cpp
    
    # 音频
    ${PPSSPP_ROOT}/UI/AudioCommon.cpp
    ${PPSSPP_ROOT}/UI/BackgroundAudio.cpp
    
    # 集成功能
    ${PPSSPP_ROOT}/UI/DiscordIntegration.cpp
    
    # IAP (应用内购买) - 包含以避免链接错误
    ${PPSSPP_ROOT}/UI/IAPScreen.cpp
    
    # ImGui 调试器
    ${PPSSPP_ROOT}/UI/ImDebugger/ImDebugger.cpp
    ${PPSSPP_ROOT}/UI/ImDebugger/ImGe.cpp
    ${PPSSPP_ROOT}/UI/ImDebugger/ImConsole.cpp
    ${PPSSPP_ROOT}/UI/ImDebugger/ImDisasmView.cpp
    ${PPSSPP_ROOT}/UI/ImDebugger/ImMemView.cpp
    ${PPSSPP_ROOT}/UI/ImDebugger/ImJitViewer.cpp
    ${PPSSPP_ROOT}/UI/ImDebugger/ImStructViewer.cpp
)

message(STATUS "UI 源文件数量: ${UI_SOURCES}")
