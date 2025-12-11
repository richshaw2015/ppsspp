# Common 库源文件列表 - 完整版
# 从主 CMakeLists.txt 提取 (约 200+ 个源文件)
# 生成时间: 2024-12-08

# Common ARM64 (鸿蒙使用)
set(CommonARM64
    ${PPSSPP_ROOT}/Common/Arm64Emitter.cpp
    ${PPSSPP_ROOT}/Common/ArmEmitter.cpp
    ${PPSSPP_ROOT}/Common/ArmCPUDetect.cpp
    # DisArm64.cpp 已在 Core 库中，不要重复添加
)

# Common 序列化
set(Common_Serialize
    ${PPSSPP_ROOT}/Common/Serialize/Serializer.cpp
)

# Common 加密
set(Common_Crypto
    ${PPSSPP_ROOT}/Common/Crypto/md5.cpp
    ${PPSSPP_ROOT}/Common/Crypto/sha1.cpp
    ${PPSSPP_ROOT}/Common/Crypto/sha256.cpp
)

# Common 数据 - 颜色和转换
set(Common_Data_Color
    ${PPSSPP_ROOT}/Common/Data/Color/RGBAUtil.cpp
    ${PPSSPP_ROOT}/Common/Data/Convert/ColorConv.cpp
    ${PPSSPP_ROOT}/Common/Data/Convert/SmallDataConvert.cpp
)

# Common 数据 - 编码
set(Common_Data_Encoding
    ${PPSSPP_ROOT}/Common/Data/Encoding/Base64.cpp
    ${PPSSPP_ROOT}/Common/Data/Encoding/Compression.cpp
    ${PPSSPP_ROOT}/Common/Data/Encoding/Utf8.cpp
)

# Common 数据 - 格式
set(Common_Data_Format
    ${PPSSPP_ROOT}/Common/Data/Format/RIFF.cpp
    ${PPSSPP_ROOT}/Common/Data/Format/IniFile.cpp
    ${PPSSPP_ROOT}/Common/Data/Format/JSONReader.cpp
    ${PPSSPP_ROOT}/Common/Data/Format/JSONWriter.cpp
    ${PPSSPP_ROOT}/Common/Data/Format/DDSLoad.cpp
    ${PPSSPP_ROOT}/Common/Data/Format/PNGLoad.cpp
    ${PPSSPP_ROOT}/Common/Data/Format/ZIMLoad.cpp
    ${PPSSPP_ROOT}/Common/Data/Format/ZIMSave.cpp
)

# Common 数据 - 哈希和文本
set(Common_Data_Other
    ${PPSSPP_ROOT}/Common/Data/Hash/Hash.cpp
    ${PPSSPP_ROOT}/Common/Data/Text/I18n.cpp
    ${PPSSPP_ROOT}/Common/Data/Text/Parsers.cpp
    ${PPSSPP_ROOT}/Common/Data/Text/WrapText.cpp
)

# Common 文件系统
set(Common_File
    ${PPSSPP_ROOT}/Common/File/VFS/VFS.cpp
    ${PPSSPP_ROOT}/Common/File/VFS/ZipFileReader.cpp
    ${PPSSPP_ROOT}/Common/File/VFS/DirectoryReader.cpp
    # TODO: 需要鸿蒙适配
    # ${PPSSPP_ROOT}/Common/File/AndroidStorage.cpp
    # ${PPSSPP_ROOT}/Common/File/AndroidContentURI.cpp
    ${PPSSPP_ROOT}/Common/File/DiskFree.cpp
    ${PPSSPP_ROOT}/Common/File/Path.cpp
    ${PPSSPP_ROOT}/Common/File/PathBrowser.cpp
    ${PPSSPP_ROOT}/Common/File/FileUtil.cpp
    ${PPSSPP_ROOT}/Common/File/DirListing.cpp
    ${PPSSPP_ROOT}/Common/File/FileDescriptor.cpp
)

# Common GPU - 基础
set(Common_GPU_Base
    ${PPSSPP_ROOT}/Common/GPU/GPUBackendCommon.cpp
    ${PPSSPP_ROOT}/Common/GPU/thin3d.cpp
    ${PPSSPP_ROOT}/Common/GPU/Shader.cpp
    ${PPSSPP_ROOT}/Common/GPU/ShaderWriter.cpp
    ${PPSSPP_ROOT}/Common/GPU/ShaderTranslation.cpp
)

# Common GPU - OpenGL (鸿蒙使用)
set(Common_GPU_OpenGL
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/GLDebugLog.cpp
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/GLSLProgram.cpp
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/gl3stub.c
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/GLFeatures.cpp
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/GLFrameData.cpp
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/thin3d_gl.cpp
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/GLMemory.cpp
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/GLRenderManager.cpp
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/GLQueueRunner.cpp
    ${PPSSPP_ROOT}/Common/GPU/OpenGL/DataFormatGL.cpp
)

# Common GPU - Vulkan (可选)
set(Common_GPU_Vulkan
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanBarrier.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanDebug.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanContext.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanDescSet.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanFramebuffer.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanImage.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanLoader.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanMemory.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanProfiler.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/thin3d_vulkan.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanRenderManager.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanQueueRunner.cpp
    ${PPSSPP_ROOT}/Common/GPU/Vulkan/VulkanFrameData.cpp
)

# Common 输入
set(Common_Input
    ${PPSSPP_ROOT}/Common/Input/GestureDetector.cpp
    ${PPSSPP_ROOT}/Common/Input/InputState.cpp
)

# Common 数学
set(Common_Math
    ${PPSSPP_ROOT}/Common/Math/fast/fast_matrix.c
    ${PPSSPP_ROOT}/Common/Math/curves.cpp
    ${PPSSPP_ROOT}/Common/Math/expression_parser.cpp
    ${PPSSPP_ROOT}/Common/Math/lin/matrix4x4.cpp
    ${PPSSPP_ROOT}/Common/Math/lin/vec3.cpp
    ${PPSSPP_ROOT}/Common/Math/math_util.cpp
    ${PPSSPP_ROOT}/Common/Math/Statistics.cpp
)

# Common 网络
set(Common_Net
    ${PPSSPP_ROOT}/Common/Net/HTTPClient.cpp
    ${PPSSPP_ROOT}/Common/Net/HTTPHeaders.cpp
    ${PPSSPP_ROOT}/Common/Net/HTTPNaettRequest.cpp
    ${PPSSPP_ROOT}/Common/Net/HTTPRequest.cpp
    ${PPSSPP_ROOT}/Common/Net/HTTPServer.cpp
    ${PPSSPP_ROOT}/Common/Net/NetBuffer.cpp
    ${PPSSPP_ROOT}/Common/Net/Resolve.cpp
    ${PPSSPP_ROOT}/Common/Net/Sinks.cpp
    ${PPSSPP_ROOT}/Common/Net/URL.cpp
    ${PPSSPP_ROOT}/Common/Net/WebsocketServer.cpp
)

# Common 性能分析
set(Common_Profiler
    ${PPSSPP_ROOT}/Common/Profiler/Profiler.cpp
)

# Common 渲染
set(Common_Render
    ${PPSSPP_ROOT}/Common/Render/AtlasGen.cpp
    ${PPSSPP_ROOT}/Common/Render/TextureAtlas.cpp
    ${PPSSPP_ROOT}/Common/Render/DrawBuffer.cpp
    ${PPSSPP_ROOT}/Common/Render/ManagedTexture.cpp
    ${PPSSPP_ROOT}/Common/Render/Text/draw_text.cpp
    # TODO: 需要鸿蒙适配
    # ${PPSSPP_ROOT}/Common/Render/Text/draw_text_android.cpp
)

# Common 系统
set(Common_System
    ${PPSSPP_ROOT}/Common/System/Display.cpp
    ${PPSSPP_ROOT}/Common/System/Request.cpp
    ${PPSSPP_ROOT}/Common/System/OSD.cpp
)

# Common 线程
set(Common_Thread
    ${PPSSPP_ROOT}/Common/Thread/ParallelLoop.cpp
    ${PPSSPP_ROOT}/Common/Thread/ThreadUtil.cpp
    ${PPSSPP_ROOT}/Common/Thread/ThreadManager.cpp
)

# Common UI
set(Common_UI
    ${PPSSPP_ROOT}/Common/UI/AsyncImageFileView.cpp
    ${PPSSPP_ROOT}/Common/UI/Root.cpp
    ${PPSSPP_ROOT}/Common/UI/Screen.cpp
    ${PPSSPP_ROOT}/Common/UI/UI.cpp
    ${PPSSPP_ROOT}/Common/UI/Context.cpp
    ${PPSSPP_ROOT}/Common/UI/IconCache.cpp
    ${PPSSPP_ROOT}/Common/UI/UIScreen.cpp
    ${PPSSPP_ROOT}/Common/UI/Tween.cpp
    ${PPSSPP_ROOT}/Common/UI/View.cpp
    ${PPSSPP_ROOT}/Common/UI/ViewGroup.cpp
    ${PPSSPP_ROOT}/Common/UI/ScrollView.cpp
    ${PPSSPP_ROOT}/Common/UI/TabHolder.cpp
    ${PPSSPP_ROOT}/Common/UI/PopupScreens.cpp
)

# Common 核心
set(Common_Core
    ${PPSSPP_ROOT}/Common/Buffer.cpp
    ${PPSSPP_ROOT}/Common/FakeCPUDetect.cpp
    ${PPSSPP_ROOT}/Common/ExceptionHandlerSetup.cpp
    ${PPSSPP_ROOT}/Common/GhidraClient.cpp
    ${PPSSPP_ROOT}/Common/Log.cpp
    ${PPSSPP_ROOT}/Common/Log/ConsoleListener.cpp
    ${PPSSPP_ROOT}/Common/Log/LogManager.cpp
    ${PPSSPP_ROOT}/Common/LogReporting.cpp
    ${PPSSPP_ROOT}/Common/MemArenaPosix.cpp
    ${PPSSPP_ROOT}/Common/MemoryUtil.cpp
    ${PPSSPP_ROOT}/Common/OSVersion.cpp
    ${PPSSPP_ROOT}/Common/StringUtils.cpp
    ${PPSSPP_ROOT}/Common/SysError.cpp
    ${PPSSPP_ROOT}/Common/TimeUtil.cpp
)

# 合并所有 Common 源文件
set(COMMON_SOURCES
    ${CommonARM64}
    ${Common_Serialize}
    ${Common_Crypto}
    ${Common_Data_Color}
    ${Common_Data_Encoding}
    ${Common_Data_Format}
    ${Common_Data_Other}
    ${Common_File}
    ${Common_GPU_Base}
    ${Common_GPU_OpenGL}
    ${Common_GPU_Vulkan}
    ${Common_Input}
    ${Common_Math}
    ${Common_Net}
    ${Common_Profiler}
    ${Common_Render}
    ${Common_System}
    ${Common_Thread}
    ${Common_UI}
    ${Common_Core}
)

# 打印统计信息
list(LENGTH COMMON_SOURCES COMMON_SOURCE_COUNT)
message(STATUS "Common 库源文件数量: ${COMMON_SOURCE_COUNT}")
