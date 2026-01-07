# 鸿蒙 PPSSPP Native 构建配置
cmake_minimum_required(VERSION 3.16)
project(ppsspp_ohos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vulkan 支持选项
option(USE_VULKAN "Enable Vulkan rendering backend" ON)

# FFmpeg 支持选项 - 用于 MPEG 视频解码
option(USE_FFMPEG "Enable FFmpeg for MPEG video decoding" ON)

# 定义编译宏
add_definitions(
    -DOHOS=1
    -D__OHOS__=1
    -DMOBILE_DEVICE=1
    -DUSING_GLES2=1
    -DUSING_EGL=1
    -D__STDC_CONSTANT_MACROS
    -DGLEW_NO_GLU
    -DHTTPS_NOT_AVAILABLE
)

# FFmpeg 相关宏定义
if(USE_FFMPEG)
    add_definitions(-DUSE_FFMPEG=1)
    message(STATUS "FFmpeg support enabled for MPEG video decoding")
endif()

# Vulkan 相关宏定义
if(USE_VULKAN)
    add_definitions(
        -DVK_USE_PLATFORM_ANDROID_KHR
    )
endif()

# 设置根目录
# 从 ohos/entry/src/main/cpp 到项目根目录需要 5 个 ../
set(PPSSPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PPSSPP_ROOT}
    ${PPSSPP_ROOT}/Common
    ${PPSSPP_ROOT}/Core
    ${PPSSPP_ROOT}/GPU
    ${PPSSPP_ROOT}/ext
    ${PPSSPP_ROOT}/ext/glew
    ${PPSSPP_ROOT}/ext/glslang
)

# NAPI 源文件
set(NAPI_SOURCES
    napi/napi_init.cpp
    napi/napi_ppsspp.cpp
)

# 平台适配源文件
set(OHOS_SOURCES
    ohos_app.cpp
    ohos_system.cpp
    ohos_audio.cpp
    ohos_audio_backend.cpp
    ohos_gl_context.cpp
    ohos_xcomponent.cpp
    ohos_input.cpp
    ohos_vibration.cpp
    ohos_content_uri.cpp
    ohos_rawfile_reader.cpp
    ohos_vr_stubs.cpp
    ohos_system_properties.cpp
    ohos_system_stubs.cpp
    ohos_offline_config.cpp
    # ohos_native_app_stubs.cpp  # 不再需要，使用真正的 UI 库
)

# Vulkan 源文件
if(USE_VULKAN)
    list(APPEND OHOS_SOURCES
        ohos_vulkan_context.cpp
    )
endif()

# 创建共享库
add_library(ppsspp_ohos SHARED
    ${NAPI_SOURCES}
    ${OHOS_SOURCES}
)

# 编译选项
target_compile_options(ppsspp_ohos PRIVATE
    -Wall
    -Wno-unused-function
    -Wno-sign-compare
    -fno-strict-aliasing
    -fno-math-errno
    -fvisibility=default  # 确保符号可见性，防止 Release 模式下符号被剥离
)

# Release 优化：使用 -O2 而不是 -O3，避免激进优化
# 同时禁用符号剥离，确保 NAPI 函数可以被正确导出
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(ppsspp_ohos PRIVATE -O2)
    # 防止链接器剥离未使用的符号
    # --no-gc-sections: 禁用垃圾回收，保留所有 section
    # --export-dynamic: 导出所有动态符号
    # -z,now: 立即绑定所有符号
    target_link_options(ppsspp_ohos PRIVATE 
        -Wl,--no-gc-sections
        -Wl,--export-dynamic
        -Wl,-z,now
    )
endif()

# 确保 NAPI 入口点符号不被剥离（无论 Debug 还是 Release）
# 使用 --undefined 强制保留关键符号
# 使用 --whole-archive 确保静态库中的所有符号都被链接
target_link_options(ppsspp_ohos PRIVATE
    -Wl,--undefined=RegisterPPSSPPModule
    -Wl,--undefined=NativeXComponentInit
    -Wl,--undefined=Init
)

# 禁用 LTO（Link Time Optimization）以防止构造函数被优化掉
# LTO 可能会在 Release 模式下移除看似未使用的代码
set_target_properties(ppsspp_ohos PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION OFF
)

# 链接鸿蒙系统库
target_link_libraries(ppsspp_ohos
    PUBLIC
    ace_napi.z
    ace_ndk.z
    hilog_ndk.z
    EGL
    GLESv3
    native_window
    native_buffer
    native_drawing
    rawfile.z
    ohaudio
)

# Vulkan 库链接
if(USE_VULKAN)
    target_link_libraries(ppsspp_ohos PUBLIC vulkan)
endif()

# FFmpeg 库链接
# 使用 OHOS 专用的预编译 FFmpeg 库（仅支持 arm64-v8a）
if(USE_FFMPEG)
    # OHOS 仅支持 arm64-v8a 架构
    set(FFMPEG_DIR ${PPSSPP_ROOT}/ffmpeg/ohos/arm64-v8a)
    
    # 添加 FFmpeg 头文件目录
    target_include_directories(ppsspp_ohos PRIVATE ${FFMPEG_DIR}/include)
    
    # 方案：使用动态库，通过 IMPORTED 目标正确链接
    # 创建 IMPORTED 目标
    add_library(ffmpeg_avformat SHARED IMPORTED)
    set_target_properties(ffmpeg_avformat PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_DIR}/lib/libavformat.so.60.3.100
    )
    
    add_library(ffmpeg_avcodec SHARED IMPORTED)
    set_target_properties(ffmpeg_avcodec PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_DIR}/lib/libavcodec.so.60.3.100
    )
    
    add_library(ffmpeg_swresample SHARED IMPORTED)
    set_target_properties(ffmpeg_swresample PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_DIR}/lib/libswresample.so.4.10.100
    )
    
    add_library(ffmpeg_swscale SHARED IMPORTED)
    set_target_properties(ffmpeg_swscale PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_DIR}/lib/libswscale.so.7.1.100
    )
    
    add_library(ffmpeg_avutil SHARED IMPORTED)
    set_target_properties(ffmpeg_avutil PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_DIR}/lib/libavutil.so.58.2.100
    )
    
    # 链接 IMPORTED 目标
    target_link_libraries(ppsspp_ohos PUBLIC
        ffmpeg_avformat
        ffmpeg_avcodec
        ffmpeg_swresample
        ffmpeg_swscale
        ffmpeg_avutil
    )
    
    # 添加链接选项以处理 PIC 问题
    target_link_options(ppsspp_ohos PRIVATE
        -Wl,--allow-shlib-undefined
        -Wl,--no-fatal-warnings
    )
    
    # 复制动态库到输出目录
    add_custom_command(TARGET ppsspp_ohos POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DIR}/lib/libavformat.so.60.3.100
            $<TARGET_FILE_DIR:ppsspp_ohos>/libavformat.so.60.3.100
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DIR}/lib/libavcodec.so.60.3.100
            $<TARGET_FILE_DIR:ppsspp_ohos>/libavcodec.so.60.3.100
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DIR}/lib/libswresample.so.4.10.100
            $<TARGET_FILE_DIR:ppsspp_ohos>/libswresample.so.4.10.100
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DIR}/lib/libswscale.so.7.1.100
            $<TARGET_FILE_DIR:ppsspp_ohos>/libswscale.so.7.1.100
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DIR}/lib/libavutil.so.58.2.100
            $<TARGET_FILE_DIR:ppsspp_ohos>/libavutil.so.58.2.100
        # 创建符号链接
        COMMAND ${CMAKE_COMMAND} -E create_symlink libavformat.so.60.3.100 $<TARGET_FILE_DIR:ppsspp_ohos>/libavformat.so.60
        COMMAND ${CMAKE_COMMAND} -E create_symlink libavcodec.so.60.3.100 $<TARGET_FILE_DIR:ppsspp_ohos>/libavcodec.so.60
        COMMAND ${CMAKE_COMMAND} -E create_symlink libswresample.so.4.10.100 $<TARGET_FILE_DIR:ppsspp_ohos>/libswresample.so.4
        COMMAND ${CMAKE_COMMAND} -E create_symlink libswscale.so.7.1.100 $<TARGET_FILE_DIR:ppsspp_ohos>/libswscale.so.7
        COMMAND ${CMAKE_COMMAND} -E create_symlink libavutil.so.58.2.100 $<TARGET_FILE_DIR:ppsspp_ohos>/libavutil.so.58
        COMMENT "Copying FFmpeg libraries to output directory"
    )
    
    message(STATUS "FFmpeg libraries linked from: ${FFMPEG_DIR}")
endif()

# ============================================================================
# 阶段 2.1: 最小依赖集成
# ============================================================================

message(STATUS "=== PPSSPP 鸿蒙适配 - 阶段 2.1: 第三方依赖集成 ===")
message(STATUS "PPSSPP_ROOT: ${PPSSPP_ROOT}")

# 检查关键目录是否存在
if(EXISTS "${PPSSPP_ROOT}/Common")
    message(STATUS "✓ Common 目录存在")
else()
    message(WARNING "✗ Common 目录不存在: ${PPSSPP_ROOT}/Common")
endif()

if(EXISTS "${PPSSPP_ROOT}/Core")
    message(STATUS "✓ Core 目录存在")
else()
    message(WARNING "✗ Core 目录不存在: ${PPSSPP_ROOT}/Core")
endif()

# ============================================================================
# 第三方依赖库配置
# ============================================================================

# 设置 ext 构建目录
set(EXT_BUILD_DIR ${CMAKE_BINARY_DIR}/ext)

# 1. CPU Features - CPU 特性检测（NEON、SSE等）
message(STATUS "添加依赖: cpu_features")
add_subdirectory(${PPSSPP_ROOT}/ext/cmake/cpu_features ${EXT_BUILD_DIR}/cpu_features EXCLUDE_FROM_ALL)

# 2. Snappy - 快速压缩/解压（用于存档）
message(STATUS "添加依赖: snappy")
# 不使用 ext/snappy 的 CMakeLists.txt，直接创建库
add_library(snappy STATIC
    ${PPSSPP_ROOT}/ext/snappy/snappy-c.cpp
    ${PPSSPP_ROOT}/ext/snappy/snappy-sinksource.cpp
    ${PPSSPP_ROOT}/ext/snappy/snappy-stubs-internal.cpp
    ${PPSSPP_ROOT}/ext/snappy/snappy.cpp
)
target_include_directories(snappy PUBLIC ${PPSSPP_ROOT}/ext/snappy)
# 创建别名以兼容主项目
add_library(Ext::Snappy ALIAS snappy)

# 3. zlib - 压缩库（libpng、libzip 依赖）
message(STATUS "添加依赖: zlib")
# 不使用 ext/zlib 的 CMakeLists.txt，直接创建库
add_library(zlibstatic STATIC
    ${PPSSPP_ROOT}/ext/zlib/adler32.c
    ${PPSSPP_ROOT}/ext/zlib/compress.c
    ${PPSSPP_ROOT}/ext/zlib/crc32.c
    ${PPSSPP_ROOT}/ext/zlib/deflate.c
    ${PPSSPP_ROOT}/ext/zlib/gzclose.c
    ${PPSSPP_ROOT}/ext/zlib/gzlib.c
    ${PPSSPP_ROOT}/ext/zlib/gzread.c
    ${PPSSPP_ROOT}/ext/zlib/gzwrite.c
    ${PPSSPP_ROOT}/ext/zlib/infback.c
    ${PPSSPP_ROOT}/ext/zlib/inffast.c
    ${PPSSPP_ROOT}/ext/zlib/inflate.c
    ${PPSSPP_ROOT}/ext/zlib/inftrees.c
    ${PPSSPP_ROOT}/ext/zlib/trees.c
    ${PPSSPP_ROOT}/ext/zlib/uncompr.c
    ${PPSSPP_ROOT}/ext/zlib/zutil.c
)
target_include_directories(zlibstatic PUBLIC ${PPSSPP_ROOT}/ext/zlib)
target_compile_options(zlibstatic PRIVATE "-O2")

# 4. libpng17 - PNG 图像加载
message(STATUS "添加依赖: libpng17")
# 手动创建 libpng17，避免复杂的配置
set(PNG_ARM_SOURCES
    ${PPSSPP_ROOT}/ext/libpng17/arm/arm_init.c
    ${PPSSPP_ROOT}/ext/libpng17/arm/filter_neon_intrinsics.c
)
add_library(png17_static STATIC
    ${PPSSPP_ROOT}/ext/libpng17/png.c
    ${PPSSPP_ROOT}/ext/libpng17/pngerror.c
    ${PPSSPP_ROOT}/ext/libpng17/pngget.c
    ${PPSSPP_ROOT}/ext/libpng17/pngmem.c
    ${PPSSPP_ROOT}/ext/libpng17/pngpread.c
    ${PPSSPP_ROOT}/ext/libpng17/pngread.c
    ${PPSSPP_ROOT}/ext/libpng17/pngrio.c
    ${PPSSPP_ROOT}/ext/libpng17/pngrtran.c
    ${PPSSPP_ROOT}/ext/libpng17/pngrutil.c
    ${PPSSPP_ROOT}/ext/libpng17/pngset.c
    ${PPSSPP_ROOT}/ext/libpng17/pngtrans.c
    ${PPSSPP_ROOT}/ext/libpng17/pngwio.c
    ${PPSSPP_ROOT}/ext/libpng17/pngwrite.c
    ${PPSSPP_ROOT}/ext/libpng17/pngwtran.c
    ${PPSSPP_ROOT}/ext/libpng17/pngwutil.c
    ${PNG_ARM_SOURCES}
)
target_include_directories(png17_static PUBLIC ${PPSSPP_ROOT}/ext/libpng17)
target_link_libraries(png17_static PUBLIC zlibstatic)
target_compile_definitions(png17_static PRIVATE PNG_ARM_NEON_OPT=0)

# 5. libzip - ZIP 文件处理
message(STATUS "添加依赖: libzip")
# 手动创建 libzip，避免复杂的配置
add_library(zip STATIC
    ${PPSSPP_ROOT}/ext/libzip/zip_add.c
    ${PPSSPP_ROOT}/ext/libzip/zip_add_dir.c
    ${PPSSPP_ROOT}/ext/libzip/zip_add_entry.c
    ${PPSSPP_ROOT}/ext/libzip/zip_algorithm_deflate.c
    ${PPSSPP_ROOT}/ext/libzip/zip_buffer.c
    ${PPSSPP_ROOT}/ext/libzip/zip_close.c
    ${PPSSPP_ROOT}/ext/libzip/zip_delete.c
    ${PPSSPP_ROOT}/ext/libzip/zip_dir_add.c
    ${PPSSPP_ROOT}/ext/libzip/zip_dirent.c
    ${PPSSPP_ROOT}/ext/libzip/zip_discard.c
    ${PPSSPP_ROOT}/ext/libzip/zip_entry.c
    ${PPSSPP_ROOT}/ext/libzip/zip_error.c
    ${PPSSPP_ROOT}/ext/libzip/zip_error_clear.c
    ${PPSSPP_ROOT}/ext/libzip/zip_error_get.c
    ${PPSSPP_ROOT}/ext/libzip/zip_error_get_sys_type.c
    ${PPSSPP_ROOT}/ext/libzip/zip_error_strerror.c
    ${PPSSPP_ROOT}/ext/libzip/zip_error_to_str.c
    ${PPSSPP_ROOT}/ext/libzip/zip_extra_field.c
    ${PPSSPP_ROOT}/ext/libzip/zip_extra_field_api.c
    ${PPSSPP_ROOT}/ext/libzip/zip_fclose.c
    ${PPSSPP_ROOT}/ext/libzip/zip_fdopen.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_add.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_error_clear.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_error_get.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_get_comment.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_get_external_attributes.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_get_offset.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_rename.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_replace.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_set_comment.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_set_encryption.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_set_external_attributes.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_set_mtime.c
    ${PPSSPP_ROOT}/ext/libzip/zip_file_strerror.c
    ${PPSSPP_ROOT}/ext/libzip/zip_fopen.c
    ${PPSSPP_ROOT}/ext/libzip/zip_fopen_encrypted.c
    ${PPSSPP_ROOT}/ext/libzip/zip_fopen_index.c
    ${PPSSPP_ROOT}/ext/libzip/zip_fopen_index_encrypted.c
    ${PPSSPP_ROOT}/ext/libzip/zip_fread.c
    ${PPSSPP_ROOT}/ext/libzip/zip_fseek.c
    ${PPSSPP_ROOT}/ext/libzip/zip_ftell.c
    ${PPSSPP_ROOT}/ext/libzip/zip_get_archive_comment.c
    ${PPSSPP_ROOT}/ext/libzip/zip_get_archive_flag.c
    ${PPSSPP_ROOT}/ext/libzip/zip_get_encryption_implementation.c
    ${PPSSPP_ROOT}/ext/libzip/zip_get_file_comment.c
    ${PPSSPP_ROOT}/ext/libzip/zip_get_name.c
    ${PPSSPP_ROOT}/ext/libzip/zip_get_num_entries.c
    ${PPSSPP_ROOT}/ext/libzip/zip_get_num_files.c
    ${PPSSPP_ROOT}/ext/libzip/zip_hash.c
    ${PPSSPP_ROOT}/ext/libzip/zip_io_util.c
    ${PPSSPP_ROOT}/ext/libzip/zip_libzip_version.c
    ${PPSSPP_ROOT}/ext/libzip/zip_memdup.c
    ${PPSSPP_ROOT}/ext/libzip/zip_name_locate.c
    ${PPSSPP_ROOT}/ext/libzip/zip_new.c
    ${PPSSPP_ROOT}/ext/libzip/zip_open.c
    ${PPSSPP_ROOT}/ext/libzip/zip_pkware.c
    ${PPSSPP_ROOT}/ext/libzip/zip_progress.c
    ${PPSSPP_ROOT}/ext/libzip/zip_rename.c
    ${PPSSPP_ROOT}/ext/libzip/zip_replace.c
    ${PPSSPP_ROOT}/ext/libzip/zip_set_archive_comment.c
    ${PPSSPP_ROOT}/ext/libzip/zip_set_archive_flag.c
    ${PPSSPP_ROOT}/ext/libzip/zip_set_default_password.c
    ${PPSSPP_ROOT}/ext/libzip/zip_set_file_comment.c
    ${PPSSPP_ROOT}/ext/libzip/zip_set_file_compression.c
    ${PPSSPP_ROOT}/ext/libzip/zip_set_name.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_accept_empty.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_begin_write.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_begin_write_cloning.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_buffer.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_call.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_close.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_commit_write.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_compress.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_crc.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_error.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_file_common.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_file_stdio.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_file_stdio_named.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_free.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_function.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_get_file_attributes.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_is_deleted.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_layered.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_open.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_pkware_decode.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_pkware_encode.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_read.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_remove.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_rollback_write.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_seek.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_seek_write.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_stat.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_supports.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_tell.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_tell_write.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_window.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_write.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_zip.c
    ${PPSSPP_ROOT}/ext/libzip/zip_source_zip_new.c
    ${PPSSPP_ROOT}/ext/libzip/zip_stat.c
    ${PPSSPP_ROOT}/ext/libzip/zip_stat_index.c
    ${PPSSPP_ROOT}/ext/libzip/zip_stat_init.c
    ${PPSSPP_ROOT}/ext/libzip/zip_strerror.c
    ${PPSSPP_ROOT}/ext/libzip/zip_string.c
    ${PPSSPP_ROOT}/ext/libzip/zip_unchange.c
    ${PPSSPP_ROOT}/ext/libzip/zip_unchange_all.c
    ${PPSSPP_ROOT}/ext/libzip/zip_unchange_archive.c
    ${PPSSPP_ROOT}/ext/libzip/zip_unchange_data.c
    ${PPSSPP_ROOT}/ext/libzip/zip_utf-8.c
    ${PPSSPP_ROOT}/ext/libzip/zip_err_str.c
    ${PPSSPP_ROOT}/ext/libzip/zip_mkstempm.c
    ${PPSSPP_ROOT}/ext/libzip/zip_random_unix.c
)
target_include_directories(zip PUBLIC ${PPSSPP_ROOT}/ext/libzip)
target_link_libraries(zip PUBLIC zlibstatic)
target_compile_definitions(zip PRIVATE HAVE_STDBOOL_H)

# 6. GLSLANG - GLSL 着色器编译
message(STATUS "添加依赖: glslang")
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${PPSSPP_ROOT}/ext/glslang ${EXT_BUILD_DIR}/glslang EXCLUDE_FROM_ALL)

# 7. SPIRV-Cross - SPIR-V 着色器转换
message(STATUS "添加依赖: SPIRV-Cross")
set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE)
add_subdirectory(${PPSSPP_ROOT}/ext/SPIRV-Cross-build ${EXT_BUILD_DIR}/SPIRV-Cross EXCLUDE_FROM_ALL)

# 8. minimp3 - MP3 解码
message(STATUS "添加依赖: minimp3")
add_subdirectory(${PPSSPP_ROOT}/ext/minimp3 ${EXT_BUILD_DIR}/minimp3 EXCLUDE_FROM_ALL)

# 9. udis86 - x86 反汇编器（用于调试）
message(STATUS "添加依赖: udis86")
add_subdirectory(${PPSSPP_ROOT}/ext/udis86 ${EXT_BUILD_DIR}/udis86 EXCLUDE_FROM_ALL)

# 10. at3_standalone - ATRAC3 音频解码
message(STATUS "添加依赖: at3_standalone")
add_subdirectory(${PPSSPP_ROOT}/ext/at3_standalone ${EXT_BUILD_DIR}/at3_standalone EXCLUDE_FROM_ALL)

# 11. libchdr - CHD 文件格式支持
message(STATUS "添加依赖: libchdr")
add_subdirectory(${PPSSPP_ROOT}/ext/libchdr-build ${EXT_BUILD_DIR}/libchdr EXCLUDE_FROM_ALL)

# 12. rcheevos - RetroAchievements 支持
message(STATUS "添加依赖: rcheevos")
add_subdirectory(${PPSSPP_ROOT}/ext/rcheevos-build ${EXT_BUILD_DIR}/rcheevos EXCLUDE_FROM_ALL)

# 13. lua - Lua 脚本支持
message(STATUS "添加依赖: lua")
add_subdirectory(${PPSSPP_ROOT}/ext/lua-build ${EXT_BUILD_DIR}/lua EXCLUDE_FROM_ALL)

# ============================================================================
# 更新包含目录
# ============================================================================

target_include_directories(ppsspp_ohos PRIVATE
    ${PPSSPP_ROOT}/ext/zlib
    ${PPSSPP_ROOT}/ext/libpng17
    ${PPSSPP_ROOT}/ext/libzip
    ${PPSSPP_ROOT}/ext/glslang
    ${PPSSPP_ROOT}/ext/SPIRV-Cross
    ${PPSSPP_ROOT}/ext/minimp3
    ${PPSSPP_ROOT}/ext/cpu_features/include
)

# ============================================================================
# 剩余 ext 库集成
# ============================================================================
# 注意：第三方依赖库已通过 Common、native、Core 间接链接，无需在此重复链接

# 14. gason - JSON 解析
add_library(gason STATIC
    ${PPSSPP_ROOT}/ext/gason/gason.cpp
    ${PPSSPP_ROOT}/ext/gason/gason.h
)

# 15. cityhash - 哈希函数
add_library(cityhash STATIC
    ${PPSSPP_ROOT}/ext/cityhash/city.cpp
    ${PPSSPP_ROOT}/ext/cityhash/city.h
)
target_include_directories(cityhash PRIVATE ${PPSSPP_ROOT}/ext/cityhash)
target_compile_options(cityhash PRIVATE "-O2")

# 16. vma - Vulkan 内存分配器
add_library(vma STATIC
    ${PPSSPP_ROOT}/ext/vma/vk_mem_alloc.cpp
    ${PPSSPP_ROOT}/ext/vma/vk_mem_alloc.h
)

# 17. kirk - PSP 加密库
add_library(kirk STATIC
    ${PPSSPP_ROOT}/ext/libkirk/AES.c
    ${PPSSPP_ROOT}/ext/libkirk/amctrl.c
    ${PPSSPP_ROOT}/ext/libkirk/SHA1.c
    ${PPSSPP_ROOT}/ext/libkirk/bn.c
    ${PPSSPP_ROOT}/ext/libkirk/ec.c
    ${PPSSPP_ROOT}/ext/libkirk/kirk_engine.c
)
target_include_directories(kirk PRIVATE ${PPSSPP_ROOT}/ext/libkirk)

# 18. sfmt19937 - 随机数生成器
add_library(sfmt19937 STATIC
    ${PPSSPP_ROOT}/ext/sfmt19937/SFMT.c
)
target_compile_definitions(sfmt19937 PRIVATE SFMT_MEXP=19937)
target_include_directories(sfmt19937 PRIVATE ${PPSSPP_ROOT}/ext/sfmt19937)

# 19. xbrz - 图像缩放算法
add_library(xbrz STATIC
    ${PPSSPP_ROOT}/ext/xbrz/xbrz.cpp
)
target_include_directories(xbrz PRIVATE ${PPSSPP_ROOT}/ext/xbrz)

# 20. xxhash - 快速哈希
add_library(xxhash STATIC
    ${PPSSPP_ROOT}/ext/xxhash.c
)

# 21. zstd - 压缩库（basis_universal 依赖）
message(STATUS "添加依赖: zstd")
add_subdirectory(${PPSSPP_ROOT}/ext/zstd-build ${EXT_BUILD_DIR}/zstd EXCLUDE_FROM_ALL)

# 22. naett - HTTP 网络库
# 暂时禁用 HTTPS 支持，因为 HarmonyOS 不支持 curl 且 JNI 实现不兼容
# message(STATUS "添加依赖: naett")
# add_subdirectory(${PPSSPP_ROOT}/ext/naett-build ${EXT_BUILD_DIR}/naett EXCLUDE_FROM_ALL)

# 23. basis_universal - 纹理压缩
add_library(basis_universal STATIC
    ${PPSSPP_ROOT}/ext/basis_universal/basisu_transcoder.cpp
)
target_include_directories(basis_universal PRIVATE ${PPSSPP_ROOT}/ext/zstd/lib)
target_link_libraries(basis_universal PRIVATE libzstd_static)

# 24. armips - ARM 汇编器（已在 ext/CMakeLists.txt 中定义）
set(ARMIPS_REGEXP OFF CACHE BOOL "" FORCE)
set(ARMIPS_LIBRARY_ONLY ON CACHE BOOL "" FORCE)
add_subdirectory(${PPSSPP_ROOT}/ext/armips ${EXT_BUILD_DIR}/armips EXCLUDE_FROM_ALL)

# 25. ImGui - UI 调试器
message(STATUS "添加依赖: imgui")
add_library(imgui STATIC
    ${PPSSPP_ROOT}/ext/imgui/imgui.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_demo.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_draw.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_extras.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_impl_thin3d.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_impl_platform.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_tables.cpp
    ${PPSSPP_ROOT}/ext/imgui/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${PPSSPP_ROOT}/ext/imgui)

message(STATUS "=== 第三方依赖集成完成 ===")
message(STATUS "已集成 24 个核心依赖库 (naett/HTTPS 暂时禁用, ImGui 已添加)")

# ============================================================================
# 阶段 2.2: Common 库集成
# ============================================================================

message(STATUS "=== 开始集成 Common 库 ===")

# 包含 Common 库源文件列表
include(${CMAKE_CURRENT_SOURCE_DIR}/common_sources.cmake)

# 创建 Common 静态库
add_library(Common STATIC ${COMMON_SOURCES})

# Common 库需要 OHOS 宏定义，否则 GLFeatures.cpp 等文件无法正确识别平台
target_compile_definitions(Common PRIVATE
    OHOS=1
    __OHOS__=1
    MOBILE_DEVICE=1
    USING_GLES2=1
    USING_EGL=1
)

target_include_directories(Common PUBLIC
    ${PPSSPP_ROOT}
    ${PPSSPP_ROOT}/Common
    ${PPSSPP_ROOT}/ext
    ${PPSSPP_ROOT}/ext/glslang
    ${PPSSPP_ROOT}/ext/zstd/lib
)

target_link_libraries(Common PUBLIC
    Ext::Snappy
    cpu_features
    zlibstatic
    png17_static
    libzstd_static
    # naett  # 暂时禁用 HTTPS 支持
)

message(STATUS "=== Common 库集成完成 ===")

# ============================================================================
# 阶段 2.3: native 库集成
# ============================================================================

message(STATUS "=== 开始集成 native 库 ===")

add_library(native STATIC
    ${PPSSPP_ROOT}/ext/jpge/jpgd.cpp
    ${PPSSPP_ROOT}/ext/jpge/jpge.cpp
)

target_link_libraries(native
    zip
    png17_static
    basis_universal
    libzstd_static
    zlibstatic
    vma
    gason
    udis86
    chdr
    # 注意：不直接链接 Common，避免重复符号
    # Common 将通过 ppsspp_ohos 的 whole-archive 链接
)

message(STATUS "=== native 库集成完成 ===")

# ============================================================================
# 阶段 2.4: Core 库集成
# ============================================================================

message(STATUS "=== 开始集成 Core 库 ===")

# 包含 Core 库源文件列表
include(${CMAKE_CURRENT_SOURCE_DIR}/core_sources.cmake)

# 创建 Core 静态库
add_library(Core STATIC ${CORE_SOURCES})

# Core 库需要 OHOS 宏定义
target_compile_definitions(Core PRIVATE
    OHOS=1
    __OHOS__=1
    MOBILE_DEVICE=1
    USING_GLES2=1
    USING_EGL=1
    USE_FFMPEG=1
#    $<$<BOOL:${USE_FFMPEG}>:USE_FFMPEG=1>
)

target_include_directories(Core PUBLIC
    ${PPSSPP_ROOT}
    ${PPSSPP_ROOT}/Core
    ${PPSSPP_ROOT}/Common
    ${PPSSPP_ROOT}/GPU
    ${PPSSPP_ROOT}/ext/libchdr/include
    ${PPSSPP_ROOT}/ext/snappy
    ${PPSSPP_ROOT}/ext/zstd/lib
)

# 添加 FFmpeg 头文件目录到 Core 库
if(USE_FFMPEG)
    target_include_directories(Core PUBLIC ${PPSSPP_ROOT}/ffmpeg/ohos/arm64-v8a/include)
endif()

target_link_libraries(Core PUBLIC
    # 注意：不直接链接 Common，避免重复符号
    # Common 将通过 ppsspp_ohos 的 whole-archive 链接
    native
    kirk
    sfmt19937
    xbrz
    xxhash
    armips
    glslang
    SPIRV
    spirv-cross-core
    spirv-cross-glsl
    chdr
    rcheevos
    lua
    at3_standalone
    minimp3
)

message(STATUS "=== Core 库集成完成 ===")

# ============================================================================
# 阶段 2.5: UI 库集成
# ============================================================================

message(STATUS "=== 开始集成 UI 库 ===")

# 包含 UI 库源文件列表
include(${CMAKE_CURRENT_SOURCE_DIR}/ui_sources.cmake)

# 创建 UI 静态库
add_library(UI STATIC ${UI_SOURCES})

# UI 库需要 OHOS 宏定义
target_compile_definitions(UI PRIVATE
    OHOS=1
    MOBILE_DEVICE=1
    USING_GLES2=1
    USING_EGL=1
)

target_include_directories(UI PUBLIC
    ${PPSSPP_ROOT}
    ${PPSSPP_ROOT}/Common
    ${PPSSPP_ROOT}/Core
    ${PPSSPP_ROOT}/UI
    ${PPSSPP_ROOT}/ext
    ${PPSSPP_ROOT}/ext/glslang
)

target_link_libraries(UI PUBLIC
    # UI 依赖 Common 和 Core
    # 注意：不直接链接，通过 ppsspp_ohos 的 whole-archive 链接
    native
    png17_static
    zlibstatic
    imgui
)

message(STATUS "=== UI 库集成完成 ===")
message(STATUS "UI 源文件数量: ${UI_SOURCES}")

# ============================================================================
# 最终链接
# ============================================================================

# 更新 ppsspp_ohos 链接
# 参考 Android 的 LOCAL_WHOLE_STATIC_LIBRARIES 策略
# 使用 WHOLE_ARCHIVE 确保 Common、Core 和 UI 的所有符号都被链接
target_link_libraries(ppsspp_ohos
    PRIVATE
    -Wl,--whole-archive
    Common  # 类似 Android 的 ppsspp_common
    Core    # 类似 Android 的 ppsspp_core
    UI      # UI 库
    -Wl,--no-whole-archive
    # Core 和 UI 的依赖会自动传递链接
)

message(STATUS "=== PPSSPP 鸿蒙适配构建配置完成 ===")
message(STATUS "已集成:")
message(STATUS "  - 24 个第三方依赖库 (HTTPS 暂时禁用, ImGui 已添加)")
message(STATUS "  - Common 库 (200+ 源文件)")
message(STATUS "  - Core 库 (1000+ 源文件)")
message(STATUS "  - UI 库 (50+ 源文件)")
message(STATUS "  - ImGui 库 (调试器支持)")
message(STATUS "  - native 库")
message(STATUS "准备编译...")
