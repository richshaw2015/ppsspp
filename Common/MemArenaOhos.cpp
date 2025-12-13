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
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include "Common/Log.h"
#include "Common/MemoryUtil.h"
#include "Common/MemArena.h"
#include "Common/StringUtils.h"

// HarmonyOS 内存分配实现
// 参考 Android 的 ashmem 机制，但使用标准的 POSIX 匿名内存映射

bool MemArena::NeedsProbing() {
	return false;
}

// Windows mappings need to be on 64K boundaries, due to Alpha legacy.
size_t MemArena::roundup(size_t x) {
	return x;
}

bool MemArena::GrabMemSpace(size_t size) {
	// HarmonyOS 使用匿名内存映射（类似 Android 的 ashmem）
	// 不需要创建文件描述符，直接使用 MAP_ANONYMOUS
	
	// 注意：我们不在这里实际分配内存，只是标记大小
	// 实际的内存映射在 CreateView 中完成
	
	// 为了兼容性，我们创建一个虚拟的 fd（-1 表示匿名映射）
	fd = -1;
	
	INFO_LOG(Log::MemMap, "HarmonyOS: Prepared anonymous memory space of size: %08x", (int)size);
	return true;
}

void MemArena::ReleaseSpace() {
	// 匿名映射不需要关闭文件描述符
	fd = -1;
}

void *MemArena::CreateView(s64 offset, size_t size, void *base) {
	// HarmonyOS 使用匿名内存映射
	// MAP_ANONYMOUS: 不需要文件支持的匿名映射
	// MAP_SHARED: 允许多个映射共享同一内存（用于镜像）
	// MAP_FIXED: 如果指定了 base，则固定在该地址
	
	int flags = MAP_ANONYMOUS | MAP_SHARED;
	if (base != nullptr) {
		flags |= MAP_FIXED;
	}
	
	void *retval = mmap(base, size, PROT_READ | PROT_WRITE, flags, -1, 0);
	
	if (retval == MAP_FAILED) {
		ERROR_LOG(Log::MemMap, "HarmonyOS mmap failed: base=%p, size=%08x, errno=%d (%s)", 
		          base, (int)size, errno, strerror(errno));
		return nullptr;
	}
	
	INFO_LOG(Log::MemMap, "HarmonyOS: Created memory view at %p (requested: %p), size: %08x", 
	         retval, base, (int)size);
	
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
