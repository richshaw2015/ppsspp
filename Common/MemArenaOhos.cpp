// Copyright (C) 2003 Dolphin Project.
// Copyright (C) 2024 PPSSPP Project - HarmonyOS adaptation

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include "ppsspp_config.h"

#if PPSSPP_PLATFORM(OHOS)

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>

// memfd_create flags - 定义以防头文件不可用
#ifndef MFD_CLOEXEC
#define MFD_CLOEXEC 0x0001U
#endif

#ifndef __NR_memfd_create
#if defined(__aarch64__)
#define __NR_memfd_create 279
#elif defined(__arm__)
#define __NR_memfd_create 385
#elif defined(__x86_64__)
#define __NR_memfd_create 319
#elif defined(__i386__)
#define __NR_memfd_create 356
#endif
#endif

#include "Common/Log.h"
#include "Common/MemoryUtil.h"
#include "Common/MemArena.h"
#include "Common/StringUtils.h"

// HarmonyOS 内存分配实现
// 使用 memfd_create 创建匿名文件描述符，支持内存镜像
// 这是 Linux 3.17+ 的特性，OHOS 基于 Linux 内核应该支持

// memfd_create 系统调用包装
static int memfd_create_wrapper(const char *name, unsigned int flags) {
#ifdef __NR_memfd_create
	return syscall(__NR_memfd_create, name, flags);
#else
	errno = ENOSYS;
	return -1;
#endif
}

bool MemArena::NeedsProbing() {
	return false;
}

// Windows mappings need to be on 64K boundaries, due to Alpha legacy.
size_t MemArena::roundup(size_t x) {
	return x;
}

// 全局变量用于存储应用缓存目录（由 OhosSystem 设置）
static std::string g_ohosAppCacheDir;

void MemArena_SetCacheDir(const char *cacheDir) {
	if (cacheDir) {
		g_ohosAppCacheDir = cacheDir;
		INFO_LOG(Log::MemMap, "HarmonyOS: Cache dir set to: %s", cacheDir);
	}
}

bool MemArena::GrabMemSpace(size_t size) {
	// 方法 1: 尝试使用 memfd_create（推荐，无需文件系统）
	fd = memfd_create_wrapper("ppsspp_mem", MFD_CLOEXEC);
	
	if (fd >= 0) {
		INFO_LOG(Log::MemMap, "HarmonyOS: Created memfd successfully (fd=%d)", fd);
	} else {
		// 方法 2: 回退到临时文件
		WARN_LOG(Log::MemMap, "HarmonyOS: memfd_create failed (errno=%d: %s), falling back to tmpfile", errno, strerror(errno));
		
		// 尝试多个位置
		const char* tmpDirs[] = {
			g_ohosAppCacheDir.c_str(),  // 应用缓存目录（首选）
			"/data/local/tmp",           // 系统临时目录
			"/tmp",                       // 标准临时目录
			nullptr
		};
		
		char tmpPath[512];
		for (int i = 0; tmpDirs[i] != nullptr; i++) {
			if (tmpDirs[i][0] == '\0') continue;  // 跳过空字符串
			
			snprintf(tmpPath, sizeof(tmpPath), "%s/ppsspp_mem_%d", tmpDirs[i], getpid());
			
			fd = open(tmpPath, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
			if (fd >= 0) {
				// 立即删除文件，但保持 fd 有效
				unlink(tmpPath);
				INFO_LOG(Log::MemMap, "HarmonyOS: Created temp file at %s (fd=%d)", tmpPath, fd);
				break;
			} else {
				WARN_LOG(Log::MemMap, "HarmonyOS: Failed to create temp file at %s: %s", tmpPath, strerror(errno));
			}
		}
		
		if (fd < 0) {
			ERROR_LOG(Log::MemMap, "HarmonyOS: All temp file methods failed");
			return false;
		}
	}
	
	// 设置文件大小
	if (ftruncate(fd, size) != 0) {
		ERROR_LOG(Log::MemMap, "HarmonyOS: ftruncate failed: %s", strerror(errno));
		close(fd);
		fd = -1;
		return false;
	}
	
	INFO_LOG(Log::MemMap, "HarmonyOS: Memory space allocated, size=%08x, fd=%d", (int)size, fd);
	return true;
}

void MemArena::ReleaseSpace() {
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}

void *MemArena::CreateView(s64 offset, size_t size, void *base) {
	// 使用 fd 和 offset 映射共享内存的视图
	// 这样多个视图可以映射到同一块物理内存（内存镜像）
	
	int flags = MAP_SHARED;
	if (base != nullptr) {
		flags |= MAP_FIXED;
	}
	
	void *retval = mmap(base, size, PROT_READ | PROT_WRITE, flags, fd, offset);
	
	if (retval == MAP_FAILED) {
		ERROR_LOG(Log::MemMap, "HarmonyOS mmap failed: base=%p, size=%08x, offset=%llx, fd=%d, errno=%d (%s)", 
		          base, (int)size, (long long)offset, fd, errno, strerror(errno));
		return nullptr;
	}
	
	INFO_LOG(Log::MemMap, "HarmonyOS: Created memory view at %p (requested: %p), size: %08x, offset: %llx", 
	         retval, base, (int)size, (long long)offset);
	
	return retval;
}

void MemArena::ReleaseView(s64 offset, void* view, size_t size) {
	if (view != nullptr) {
		munmap(view, size);
	}
}

u8* MemArena::Find4GBBase() {
#if PPSSPP_ARCH(64BIT)
	// 64位架构：尝试分配 8GB 并对齐到 4GB 边界
	// 这样可以使用 movk 指令优化（ARM64）
	const uint64_t EIGHT_GIGS = 0x200000000ULL;
	void *base = mmap(nullptr, EIGHT_GIGS, PROT_NONE, 
	                  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	
	if (base && base != MAP_FAILED) {
		INFO_LOG(Log::MemMap, "HarmonyOS 64-bit: Got base at %p", base);
		uint64_t aligned_base = ((uint64_t)base + 0xFFFFFFFF) & ~0xFFFFFFFFULL;
		INFO_LOG(Log::MemMap, "HarmonyOS 64-bit: Aligned base: %p", (void *)aligned_base);
		munmap(base, EIGHT_GIGS);
		return reinterpret_cast<u8 *>(aligned_base);
	} else {
		// 失败时使用硬编码地址
		u8 *hardcoded_ptr = reinterpret_cast<u8*>(0x2300000000ULL);
		WARN_LOG(Log::MemMap, "HarmonyOS 64-bit: Failed to map 8GB (%s), using hardcoded pointer %p", 
		         strerror(errno), hardcoded_ptr);
		return hardcoded_ptr;
	}
#else
	// 32位架构：分配 256MB 用于地址掩码
	const size_t size = 0x10000000;  // 256 MB
	void *base = mmap(nullptr, size, PROT_NONE, 
	                  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	
	if (base == MAP_FAILED) {
		ERROR_LOG(Log::MemMap, "HarmonyOS 32-bit: Failed to map 256 MB: %s", strerror(errno));
		return nullptr;
	}
	
	INFO_LOG(Log::MemMap, "HarmonyOS 32-bit: Got base at %p", base);
	munmap(base, size);
	return static_cast<u8*>(base);
#endif
}

#endif  // PPSSPP_PLATFORM(OHOS)
