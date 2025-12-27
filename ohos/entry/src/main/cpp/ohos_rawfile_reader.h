// PPSSPP - OpenHarmony Platform Adaptation
// Copyright (c) 2024-2025 richshaw
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License 2.0 for more details.
//
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

/**
 * OHOS Rawfile Reader - 直接读取 rawfile 资源
 * 类似于 Android 的 AssetReader
 */

#pragma once

#include "Common/File/VFS/VFS.h"
#include <rawfile/raw_file_manager.h>
#include <string>
#include <vector>

class OhosRawfileReader : public VFSBackend {
public:
    explicit OhosRawfileReader(NativeResourceManager* resourceManager);
    ~OhosRawfileReader() override;
    
    // VFS 接口实现 - 文件信息
    bool GetFileListing(const char *path, std::vector<File::FileInfo> *listing, const char *filter = nullptr) override;
    bool GetFileInfo(const char *path, File::FileInfo *info) override;
    bool GetFileInfo(VFSFileReference *vfsReference, File::FileInfo *fileInfo) override;
    std::string toString() const override { return "OhosRawfileReader"; }
    
    // VFS 接口实现 - 文件引用
    VFSFileReference *GetFile(const char *path) override;
    void ReleaseFile(VFSFileReference *vfsReference) override;
    
    // VFS 接口实现 - 文件读取
    uint8_t *ReadFile(const char *path, size_t *size) override;
    VFSOpenFile *OpenFileForRead(VFSFileReference *vfsReference, size_t *size) override;
    void Rewind(VFSOpenFile *vfsOpenFile) override;
    size_t Read(VFSOpenFile *vfsOpenFile, void *buffer, size_t length) override;
    void CloseFile(VFSOpenFile *vfsOpenFile) override;
    
    // 辅助函数（非虚函数）
    bool ReadFileToString(const char *path, std::string *contents, bool quiet = false);
    
private:
    NativeResourceManager* resourceManager_;
    
    // 内部辅助函数
    std::string GetFullPath(const char *path) const;
    bool FileExists(const char *path) const;
};

