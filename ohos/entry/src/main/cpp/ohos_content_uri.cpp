// HarmonyOS Content URI 适配
// 替代 Android 的 ContentURI 实现

#include "Common/File/AndroidContentURI.h"
#include <string>

// 为 HarmonyOS 提供 AndroidContentURI 的空实现
// 这些函数在 Android 上用于处理 content:// URI
// HarmonyOS 暂时不需要这个功能

bool AndroidContentURI::Parse(std::string_view uri) {
    // 返回 false 表示解析失败
    return false;
}

AndroidContentURI AndroidContentURI::WithComponent(std::string_view component) {
    return *this;
}

std::string AndroidContentURI::ToString() const {
    return "";
}

AndroidContentURI AndroidContentURI::WithExtraExtension(std::string_view ext) {
    return *this;
}

AndroidContentURI AndroidContentURI::WithReplacedExtension(
    const std::string &oldExtension,
    const std::string &newExtension) const {
    return *this;
}

AndroidContentURI AndroidContentURI::WithReplacedExtension(
    const std::string &newExtension) const {
    return *this;
}

std::string AndroidContentURI::GetFileExtension() const {
    return "";
}

std::string AndroidContentURI::GetLastPart() const {
    return "";
}

bool AndroidContentURI::NavigateUp() {
    return false;
}

bool AndroidContentURI::CanNavigateUp() const {
    return false;
}

AndroidContentURI AndroidContentURI::WithRootFilePath(const std::string &path) {
    return AndroidContentURI();
}

bool AndroidContentURI::ComputePathTo(
    const AndroidContentURI &other,
    std::string &path) const {
    return false;
}
