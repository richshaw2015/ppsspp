# Core 库源文件列表 - 完整版
# 从主 CMakeLists.txt 提取 (约 1000+ 个源文件)
# 生成时间: 2024-12-08

# CommonJIT - JIT 通用代码
set(CommonJIT
    ${PPSSPP_ROOT}/Core/MIPS/JitCommon/JitCommon.cpp
    ${PPSSPP_ROOT}/Core/MIPS/JitCommon/JitBlockCache.cpp
    ${PPSSPP_ROOT}/Core/MIPS/JitCommon/JitState.cpp
)

# CoreExtra - MIPS IR (中间表示)
set(CoreExtra_IR
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRAnalysis.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRCompALU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRCompBranch.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRCompFPU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRCompLoadStore.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRCompVFPU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRFrontend.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRInst.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRInterpreter.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRJit.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRNativeCommon.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRPassSimplify.cpp
    ${PPSSPP_ROOT}/Core/MIPS/IR/IRRegCache.cpp
)

# CoreExtra - MIPS ARM64 JIT (鸿蒙使用)
set(CoreExtra_ARM64
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64Asm.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64CompALU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64CompBranch.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64CompFPU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64CompLoadStore.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64CompVFPU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64CompReplace.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64Jit.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64RegCache.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64RegCacheFPU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64IRAsm.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64IRCompALU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64IRCompBranch.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64IRCompFPU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64IRCompLoadStore.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64IRCompSystem.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64IRCompVec.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64IRJit.cpp
    ${PPSSPP_ROOT}/Core/MIPS/ARM64/Arm64IRRegCache.cpp
    ${PPSSPP_ROOT}/GPU/Common/VertexDecoderArm64.cpp
    ${PPSSPP_ROOT}/Core/Util/DisArm64.cpp
)

# GPU 实现 - OpenGL ES (鸿蒙使用)
set(GPU_GLES
    ${PPSSPP_ROOT}/GPU/GLES/StencilBufferGLES.cpp
    ${PPSSPP_ROOT}/GPU/GLES/GPU_GLES.cpp
    ${PPSSPP_ROOT}/GPU/GLES/FragmentTestCacheGLES.cpp
    ${PPSSPP_ROOT}/GPU/GLES/FramebufferManagerGLES.cpp
    ${PPSSPP_ROOT}/GPU/GLES/ShaderManagerGLES.cpp
    ${PPSSPP_ROOT}/GPU/GLES/StateMappingGLES.cpp
    ${PPSSPP_ROOT}/GPU/GLES/TextureCacheGLES.cpp
    ${PPSSPP_ROOT}/GPU/GLES/DrawEngineGLES.cpp
)

# GPU 实现 - Vulkan (可选)
set(GPU_VULKAN
    ${PPSSPP_ROOT}/GPU/Vulkan/DebugVisVulkan.cpp
    ${PPSSPP_ROOT}/GPU/Vulkan/DrawEngineVulkan.cpp
    ${PPSSPP_ROOT}/GPU/Vulkan/FramebufferManagerVulkan.cpp
    ${PPSSPP_ROOT}/GPU/Vulkan/GPU_Vulkan.cpp
    ${PPSSPP_ROOT}/GPU/Vulkan/PipelineManagerVulkan.cpp
    ${PPSSPP_ROOT}/GPU/Vulkan/ShaderManagerVulkan.cpp
    ${PPSSPP_ROOT}/GPU/Vulkan/StateMappingVulkan.cpp
    ${PPSSPP_ROOT}/GPU/Vulkan/TextureCacheVulkan.cpp
    ${PPSSPP_ROOT}/GPU/Vulkan/VulkanUtil.cpp
)

# GPU 通用代码
set(GPU_COMMON
    ${PPSSPP_ROOT}/GPU/Common/Draw2D.cpp
    ${PPSSPP_ROOT}/GPU/Common/DepthBufferCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/DepthRaster.cpp
    ${PPSSPP_ROOT}/GPU/Common/TextureShaderCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/DepalettizeShaderCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/FragmentShaderGenerator.cpp
    ${PPSSPP_ROOT}/GPU/Common/VertexShaderGenerator.cpp
    ${PPSSPP_ROOT}/GPU/Common/GeometryShaderGenerator.cpp
    ${PPSSPP_ROOT}/GPU/Common/FramebufferManagerCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/GPUDebugInterface.cpp
    ${PPSSPP_ROOT}/GPU/Common/GPUStateUtils.cpp
    ${PPSSPP_ROOT}/GPU/Common/DrawEngineCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/PresentationCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/ReinterpretFramebuffer.cpp
    ${PPSSPP_ROOT}/GPU/Common/ShaderId.cpp
    ${PPSSPP_ROOT}/GPU/Common/ShaderUniforms.cpp
    ${PPSSPP_ROOT}/GPU/Common/ShaderCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/SplineCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/StencilCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/SoftwareTransformCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/VertexDecoderCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/VertexDecoderHandwritten.cpp
    ${PPSSPP_ROOT}/GPU/Common/TransformCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/IndexGenerator.cpp
    ${PPSSPP_ROOT}/GPU/Common/TextureDecoder.cpp
    ${PPSSPP_ROOT}/GPU/Common/TextureCacheCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/TextureScalerCommon.cpp
    ${PPSSPP_ROOT}/GPU/Common/PostShader.cpp
    ${PPSSPP_ROOT}/GPU/Common/TextureReplacer.cpp
    ${PPSSPP_ROOT}/GPU/Common/ReplacedTexture.cpp
)

# GPU 调试器
set(GPU_DEBUGGER
    ${PPSSPP_ROOT}/GPU/Debugger/Breakpoints.cpp
    ${PPSSPP_ROOT}/GPU/Debugger/Debugger.cpp
    ${PPSSPP_ROOT}/GPU/Debugger/GECommandTable.cpp
    ${PPSSPP_ROOT}/GPU/Debugger/Playback.cpp
    ${PPSSPP_ROOT}/GPU/Debugger/Record.cpp
    ${PPSSPP_ROOT}/GPU/Debugger/State.cpp
    ${PPSSPP_ROOT}/GPU/Debugger/Stepping.cpp
)

# GPU 基础
set(GPU_BASE
    ${PPSSPP_ROOT}/GPU/GeConstants.cpp
    ${PPSSPP_ROOT}/GPU/GeDisasm.cpp
    ${PPSSPP_ROOT}/GPU/GPU.cpp
    ${PPSSPP_ROOT}/GPU/GPUCommon.cpp
    ${PPSSPP_ROOT}/GPU/GPUCommonHW.cpp
    ${PPSSPP_ROOT}/GPU/GPUState.cpp
    ${PPSSPP_ROOT}/GPU/Math3D.cpp
)

# GPU 软件渲染
set(GPU_SOFTWARE
    ${PPSSPP_ROOT}/GPU/Software/BinManager.cpp
    ${PPSSPP_ROOT}/GPU/Software/Clipper.cpp
    ${PPSSPP_ROOT}/GPU/Software/DrawPixel.cpp
    ${PPSSPP_ROOT}/GPU/Software/FuncId.cpp
    ${PPSSPP_ROOT}/GPU/Software/Lighting.cpp
    ${PPSSPP_ROOT}/GPU/Software/Rasterizer.cpp
    ${PPSSPP_ROOT}/GPU/Software/RasterizerRectangle.cpp
    ${PPSSPP_ROOT}/GPU/Software/RasterizerRegCache.cpp
    ${PPSSPP_ROOT}/GPU/Software/Sampler.cpp
    ${PPSSPP_ROOT}/GPU/Software/SoftGpu.cpp
    ${PPSSPP_ROOT}/GPU/Software/TransformUnit.cpp
)

# 合并所有 GPU 源文件
set(GPU_SOURCES
    ${GPU_GLES}
    ${GPU_VULKAN}
    ${GPU_COMMON}
    ${GPU_DEBUGGER}
    ${GPU_BASE}
    ${GPU_SOFTWARE}
)

# Core 核心系统
set(CORE_SYSTEM
    ${PPSSPP_ROOT}/Core/Config.cpp
    ${PPSSPP_ROOT}/Core/ConfigSettings.cpp
    ${PPSSPP_ROOT}/Core/ControlMapper.cpp
    ${PPSSPP_ROOT}/Core/Core.cpp
    ${PPSSPP_ROOT}/Core/Compatibility.cpp
    ${PPSSPP_ROOT}/Core/CoreTiming.cpp
    ${PPSSPP_ROOT}/Core/CwCheat.cpp
    ${PPSSPP_ROOT}/Core/FrameTiming.cpp
    ${PPSSPP_ROOT}/Core/HDRemaster.cpp
    ${PPSSPP_ROOT}/Core/Instance.cpp
    ${PPSSPP_ROOT}/Core/KeyMap.cpp
    ${PPSSPP_ROOT}/Core/KeyMapDefaults.cpp
    ${PPSSPP_ROOT}/Core/LuaContext.cpp
    ${PPSSPP_ROOT}/Core/RetroAchievements.cpp
    ${PPSSPP_ROOT}/Core/TiltEventProcessor.cpp
    ${PPSSPP_ROOT}/Core/WebServer.cpp
)

# Core 调试器
set(CORE_DEBUGGER
    ${PPSSPP_ROOT}/Core/Debugger/Breakpoints.cpp
    ${PPSSPP_ROOT}/Core/Debugger/MemBlockInfo.cpp
    ${PPSSPP_ROOT}/Core/Debugger/SymbolMap.cpp
    ${PPSSPP_ROOT}/Core/Debugger/DisassemblyManager.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/BreakpointSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/CPUCoreSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/DisasmSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/GameBroadcaster.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/GameSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/ClientConfigSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/GPUBufferSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/GPURecordSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/GPUStatsSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/HLESubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/InputBroadcaster.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/InputSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/LogBroadcaster.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/MemoryInfoSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/MemorySubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/ReplaySubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/SteppingBroadcaster.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/SteppingSubscriber.cpp
    ${PPSSPP_ROOT}/Core/Debugger/WebSocket/WebSocketUtils.cpp
)

# Core 对话框
set(CORE_DIALOG
    ${PPSSPP_ROOT}/Core/Dialog/PSPDialog.cpp
    ${PPSSPP_ROOT}/Core/Dialog/PSPGamedataInstallDialog.cpp
    ${PPSSPP_ROOT}/Core/Dialog/PSPMsgDialog.cpp
    ${PPSSPP_ROOT}/Core/Dialog/PSPNetconfDialog.cpp
    ${PPSSPP_ROOT}/Core/Dialog/PSPNpSigninDialog.cpp
    ${PPSSPP_ROOT}/Core/Dialog/PSPOskDialog.cpp
    ${PPSSPP_ROOT}/Core/Dialog/PSPOskConstants.cpp
    ${PPSSPP_ROOT}/Core/Dialog/PSPPlaceholderDialog.cpp
    ${PPSSPP_ROOT}/Core/Dialog/PSPSaveDialog.cpp
    ${PPSSPP_ROOT}/Core/Dialog/PSPScreenshotDialog.cpp
    ${PPSSPP_ROOT}/Core/Dialog/SavedataParam.cpp
)

# Core ELF 加载器
set(CORE_ELF
    ${PPSSPP_ROOT}/Core/ELF/ElfReader.cpp
    ${PPSSPP_ROOT}/Core/ELF/PBPReader.cpp
    ${PPSSPP_ROOT}/Core/ELF/PrxDecrypter.cpp
    ${PPSSPP_ROOT}/Core/ELF/ParamSFO.cpp
)

# Core 文件系统
set(CORE_FILESYSTEMS
    ${PPSSPP_ROOT}/Core/FileSystems/tlzrc.cpp
    ${PPSSPP_ROOT}/Core/FileSystems/BlobFileSystem.cpp
    ${PPSSPP_ROOT}/Core/FileSystems/BlockDevices.cpp
    ${PPSSPP_ROOT}/Core/FileSystems/DirectoryFileSystem.cpp
    ${PPSSPP_ROOT}/Core/FileSystems/FileSystem.cpp
    ${PPSSPP_ROOT}/Core/FileSystems/ISOFileSystem.cpp
    ${PPSSPP_ROOT}/Core/FileSystems/MetaFileSystem.cpp
    ${PPSSPP_ROOT}/Core/FileSystems/VirtualDiscFileSystem.cpp
)

# Core 字体
set(CORE_FONT
    ${PPSSPP_ROOT}/Core/Font/PGF.cpp
)

# Core HLE (高级模拟) - 基础
set(CORE_HLE_BASE
    ${PPSSPP_ROOT}/Core/HLE/HLE.cpp
    ${PPSSPP_ROOT}/Core/HLE/ReplaceTables.cpp
    ${PPSSPP_ROOT}/Core/HLE/HLEHelperThread.cpp
    ${PPSSPP_ROOT}/Core/HLE/HLETables.cpp
    ${PPSSPP_ROOT}/Core/HLE/KUBridge.cpp
    ${PPSSPP_ROOT}/Core/HLE/Plugins.cpp
    ${PPSSPP_ROOT}/Core/HLE/__sceAudio.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceAdler.cpp
    ${PPSSPP_ROOT}/Core/HLE/AtracCtx.cpp
    ${PPSSPP_ROOT}/Core/HLE/AtracCtx2.cpp
    ${PPSSPP_ROOT}/Core/HLE/NetInetConstants.cpp
    ${PPSSPP_ROOT}/Core/HLE/SocketManager.cpp
)

# Core HLE - PSP 系统调用 (约 100 个文件)
set(CORE_HLE_SCE
    ${PPSSPP_ROOT}/Core/HLE/sceAtrac.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceAudio.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceAudiocodec.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceAudioRouting.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceCcc.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceChnnlsv.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceCtrl.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceDeflt.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceDisplay.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceDmac.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceG729.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceGameUpdate.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceGe.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceFont.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceHeap.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceHprm.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceHttp.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceImpose.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceIo.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceJpeg.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernel.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelAlarm.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelEventFlag.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelHeap.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelInterrupt.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelMbx.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelMemory.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelModule.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelMsgPipe.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelMutex.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelSemaphore.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelThread.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelTime.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceKernelVTimer.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceMpeg.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceNet.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceNet_lib.cpp
    ${PPSSPP_ROOT}/Core/HLE/NetAdhocCommon.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceNetAdhoc.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceNetAdhocMatching.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceNetInet.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceNetApctl.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceNetResolver.cpp
    ${PPSSPP_ROOT}/Core/HLE/proAdhoc.cpp
    ${PPSSPP_ROOT}/Core/HLE/proAdhocServer.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceOpenPSID.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceP3da.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceMt19937.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceMd5.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceAac.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceMp4.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceMp3.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceParseHttp.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceParseUri.cpp
    ${PPSSPP_ROOT}/Core/HLE/scePower.cpp
    ${PPSSPP_ROOT}/Core/HLE/scePsmf.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceReg.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceRtc.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceSas.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceSfmt19937.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceSha256.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceSircs.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceSsl.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceUmd.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceUsb.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceUsbAcc.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceUsbCam.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceUsbGps.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceUsbMic.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceUtility.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceVaudio.cpp
    ${PPSSPP_ROOT}/Core/HLE/scePspNpDrm_user.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceNp.cpp
    ${PPSSPP_ROOT}/Core/HLE/sceNp2.cpp
    ${PPSSPP_ROOT}/Core/HLE/scePauth.cpp
)

# Core 硬件模拟
set(CORE_HW
    ${PPSSPP_ROOT}/Core/HW/SimpleAudioDec.cpp
    ${PPSSPP_ROOT}/Core/HW/Atrac3Standalone.cpp
    ${PPSSPP_ROOT}/Core/HW/AsyncIOManager.cpp
    ${PPSSPP_ROOT}/Core/HW/BufferQueue.cpp
    ${PPSSPP_ROOT}/Core/HW/Camera.cpp
    ${PPSSPP_ROOT}/Core/HW/Display.cpp
    ${PPSSPP_ROOT}/Core/HW/GranularMixer.cpp
    ${PPSSPP_ROOT}/Core/HW/MediaEngine.cpp
    ${PPSSPP_ROOT}/Core/HW/MpegDemux.cpp
    ${PPSSPP_ROOT}/Core/HW/MemoryStick.cpp
    ${PPSSPP_ROOT}/Core/HW/SasAudio.cpp
    ${PPSSPP_ROOT}/Core/HW/SasReverb.cpp
    ${PPSSPP_ROOT}/Core/HW/StereoResampler.cpp
)

# Core 文件加载器
set(CORE_LOADERS
    ${PPSSPP_ROOT}/Core/Loaders.cpp
    ${PPSSPP_ROOT}/Core/FileLoaders/CachingFileLoader.cpp
    ${PPSSPP_ROOT}/Core/FileLoaders/DiskCachingFileLoader.cpp
    ${PPSSPP_ROOT}/Core/FileLoaders/HTTPFileLoader.cpp
    ${PPSSPP_ROOT}/Core/FileLoaders/LocalFileLoader.cpp
    ${PPSSPP_ROOT}/Core/FileLoaders/RamCachingFileLoader.cpp
    ${PPSSPP_ROOT}/Core/FileLoaders/RetryingFileLoader.cpp
    ${PPSSPP_ROOT}/Core/FileLoaders/ZipFileLoader.cpp
)

# Core MIPS 基础
set(CORE_MIPS
    ${PPSSPP_ROOT}/Core/MIPS/MIPS.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSAnalyst.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSCodeUtils.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSDebugInterface.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSDis.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSDisVFPU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSInt.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSIntVFPU.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSStackWalk.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSTables.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSVFPUUtils.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSVFPUFallbacks.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSAsm.cpp
    ${PPSSPP_ROOT}/Core/MIPS/MIPSTracer.cpp
)

# Core 内存和系统
set(CORE_MEMORY
    ${PPSSPP_ROOT}/Core/MemFault.cpp
    ${PPSSPP_ROOT}/Core/MemMap.cpp
    ${PPSSPP_ROOT}/Core/MemMapFunctions.cpp
    ${PPSSPP_ROOT}/Core/PSPLoaders.cpp
    ${PPSSPP_ROOT}/Core/Reporting.cpp
    ${PPSSPP_ROOT}/Core/Replay.cpp
    ${PPSSPP_ROOT}/Core/SaveState.cpp
    ${PPSSPP_ROOT}/Core/Screenshot.cpp
    ${PPSSPP_ROOT}/Core/System.cpp
)

# Core 工具类
set(CORE_UTIL
    ${PPSSPP_ROOT}/Core/Util/AtracTrack.cpp
    ${PPSSPP_ROOT}/Core/Util/AudioFormat.cpp
    ${PPSSPP_ROOT}/Core/Util/GameManager.cpp
    ${PPSSPP_ROOT}/Core/Util/MemStick.cpp
    ${PPSSPP_ROOT}/Core/Util/GameDB.cpp
    ${PPSSPP_ROOT}/Core/Util/PathUtil.cpp
    ${PPSSPP_ROOT}/Core/Util/PortManager.cpp
    ${PPSSPP_ROOT}/Core/Util/BlockAllocator.cpp
    ${PPSSPP_ROOT}/Core/Util/PPGeDraw.cpp
    ${PPSSPP_ROOT}/Core/Util/RecentFiles.cpp
)

# ext 反汇编器
set(EXT_DISASM
    ${PPSSPP_ROOT}/ext/disarm.cpp
    ${PPSSPP_ROOT}/ext/riscv-disas.cpp
    ${PPSSPP_ROOT}/ext/loongarch-disasm.cpp
)

# 合并所有 Core 源文件
set(CORE_SOURCES
    ${CommonJIT}
    ${CoreExtra_IR}
    ${CoreExtra_ARM64}
    ${GPU_SOURCES}
    ${CORE_SYSTEM}
    ${CORE_DEBUGGER}
    ${CORE_DIALOG}
    ${CORE_ELF}
    ${CORE_FILESYSTEMS}
    ${CORE_FONT}
    ${CORE_HLE_BASE}
    ${CORE_HLE_SCE}
    ${CORE_HW}
    ${CORE_LOADERS}
    ${CORE_MIPS}
    ${CORE_MEMORY}
    ${CORE_UTIL}
    ${EXT_DISASM}
)

# 打印统计信息
list(LENGTH CORE_SOURCES CORE_SOURCE_COUNT)
message(STATUS "Core 库源文件数量: ${CORE_SOURCE_COUNT}")
