/**
 * OHOS Rawfile Reader 实现
 */

#include "ohos_rawfile_reader.h"
#include "ohos_hilog.h"
#include "Common/Log.h"
#include "Common/StringUtils.h"
#include <cstring>

#define RAWFILE_TAG "PPSSPP_Rawfile"

OhosRawfileReader::OhosRawfileReader(NativeResourceManager* resourceManager)
    : resourceManager_(resourceManager) {
    OHOS_LOGI(RAWFILE_TAG, "OhosRawfileReader created");
}

OhosRawfileReader::~OhosRawfileReader() {
    // ResourceManager 由外部管理，不在这里释放
    OHOS_LOGI(RAWFILE_TAG, "OhosRawfileReader destroyed");
}

std::string OhosRawfileReader::GetFullPath(const char *path) const {
    // rawfile 路径格式：assets/xxx
    if (strncmp(path, "assets/", 7) == 0) {
        return std::string(path);
    }
    return std::string("assets/") + path;
}

bool OhosRawfileReader::FileExists(const char *path) const {
    if (!resourceManager_) {
        return false;
    }
    
    std::string fullPath = GetFullPath(path);
    RawFile* rawFile = OH_ResourceManager_OpenRawFile(resourceManager_, fullPath.c_str());
    if (rawFile) {
        OH_ResourceManager_CloseRawFile(rawFile);
        return true;
    }
    return false;
}

bool OhosRawfileReader::GetFileInfo(const char *path, File::FileInfo *info) {
    if (!resourceManager_ || !info) {
        return false;
    }
    
    std::string fullPath = GetFullPath(path);
    RawFile* rawFile = OH_ResourceManager_OpenRawFile(resourceManager_, fullPath.c_str());
    if (!rawFile) {
        return false;
    }
    
    // 获取文件大小
    long size = OH_ResourceManager_GetRawFileSize(rawFile);
    OH_ResourceManager_CloseRawFile(rawFile);
    
    if (size < 0) {
        return false;
    }
    
    // 填充文件信息
    info->name = path;
    info->fullName = Path(path);
    info->size = size;
    info->isDirectory = false;
    info->isWritable = false;
    info->exists = true;
    
    return true;
}

bool OhosRawfileReader::GetFileInfo(VFSFileReference *vfsReference, File::FileInfo *fileInfo) {
    // VFSFileReference 包含路径信息
    // 暂时不实现，因为我们主要使用 GetFileInfo(const char *path, ...)
    OHOS_LOGW(RAWFILE_TAG, "GetFileInfo(VFSFileReference) not implemented");
    return false;
}

uint8_t *OhosRawfileReader::ReadFile(const char *path, size_t *size) {
    if (!resourceManager_) {
        OHOS_LOGE(RAWFILE_TAG, "ResourceManager is null");
        return nullptr;
    }
    
    std::string fullPath = GetFullPath(path);
    RawFile* rawFile = OH_ResourceManager_OpenRawFile(resourceManager_, fullPath.c_str());
    if (!rawFile) {
        OHOS_LOGW(RAWFILE_TAG, "Failed to open rawfile: %{public}s", fullPath.c_str());
        return nullptr;
    }
    
    // 获取文件大小
    long fileSize = OH_ResourceManager_GetRawFileSize(rawFile);
    if (fileSize <= 0) {
        OHOS_LOGE(RAWFILE_TAG, "Invalid file size: %{public}ld for %{public}s", fileSize, fullPath.c_str());
        OH_ResourceManager_CloseRawFile(rawFile);
        return nullptr;
    }
    
    // 分配缓冲区
    uint8_t *buffer = new uint8_t[fileSize];
    if (!buffer) {
        OHOS_LOGE(RAWFILE_TAG, "Failed to allocate buffer for %{public}s", fullPath.c_str());
        OH_ResourceManager_CloseRawFile(rawFile);
        return nullptr;
    }
    
    // 读取文件
    int bytesRead = OH_ResourceManager_ReadRawFile(rawFile, buffer, fileSize);
    OH_ResourceManager_CloseRawFile(rawFile);
    
    if (bytesRead != fileSize) {
        OHOS_LOGE(RAWFILE_TAG, "Read size mismatch: expected %{public}ld, got %{public}d for %{public}s", 
                  fileSize, bytesRead, fullPath.c_str());
        delete[] buffer;
        return nullptr;
    }
    
    if (size) {
        *size = fileSize;
    }
    
    OHOS_LOGD(RAWFILE_TAG, "Read file: %{public}s (%{public}ld bytes)", fullPath.c_str(), fileSize);
    return buffer;
}

bool OhosRawfileReader::ReadFileToString(const char *path, std::string *contents, bool quiet) {
    if (!contents) {
        return false;
    }
    
    size_t size = 0;
    uint8_t *data = ReadFile(path, &size);
    if (!data) {
        if (!quiet) {
            OHOS_LOGW(RAWFILE_TAG, "Failed to read file to string: %{public}s", path);
        }
        return false;
    }
    
    contents->assign((const char*)data, size);
    delete[] data;
    return true;
}

// VFS 文件引用实现
VFSFileReference *OhosRawfileReader::GetFile(const char *path) {
    // 简单实现：不需要复杂的文件引用，直接返回 nullptr
    // 因为我们主要使用 ReadFile 直接读取
    return nullptr;
}

void OhosRawfileReader::ReleaseFile(VFSFileReference *vfsReference) {
    // 不需要释放，因为 GetFile 返回 nullptr
}

// VFS 流式读取实现
VFSOpenFile *OhosRawfileReader::OpenFileForRead(VFSFileReference *vfsReference, size_t *size) {
    // 暂不实现流式读取，因为 PPSSPP 主要使用 ReadFile 一次性读取
    OHOS_LOGW(RAWFILE_TAG, "OpenFileForRead not implemented");
    return nullptr;
}

void OhosRawfileReader::Rewind(VFSOpenFile *vfsOpenFile) {
    // 暂不实现
}

size_t OhosRawfileReader::Read(VFSOpenFile *vfsOpenFile, void *buffer, size_t length) {
    // 暂不实现
    return 0;
}

void OhosRawfileReader::CloseFile(VFSOpenFile *vfsOpenFile) {
    // 暂不实现
}

bool OhosRawfileReader::GetFileListing(const char *path, std::vector<File::FileInfo> *listing, const char *filter) {
    // OHOS rawfile API 不支持列出目录内容
    // 我们需要使用预定义的文件列表
    
    OHOS_LOGW(RAWFILE_TAG, "GetFileListing not fully supported for path: %{public}s", path);
    
    std::string pathStr(path);
    
    // 对于常用目录，返回预定义的文件列表
    if (pathStr == "lang" || pathStr == "assets/lang") {
        // 语言文件列表 - 只保留中英文
        static const char* LANG_FILES[] = {
            "en_US.ini",
            "zh_CN.ini",
        };
        
        for (const char* filename : LANG_FILES) {
            // 应用过滤器
            if (filter && !strstr(filename, filter)) {
                continue;
            }
            
            std::string fullPath = pathStr + "/" + filename;
            File::FileInfo info;
            if (GetFileInfo(fullPath.c_str(), &info)) {
                // 只保留文件名，不包含路径
                info.name = filename;
                listing->push_back(info);
            }
        }
        
        return !listing->empty();
    }
    
    // 其他目录暂不支持
    return false;
}

