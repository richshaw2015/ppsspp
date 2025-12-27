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
