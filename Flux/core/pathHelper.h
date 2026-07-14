#pragma once
#include <filesystem>
#include <string>

namespace Flux {
    class PathHelper {
    public:
        static std::string GetAssetPath(const std::string& relativePath);
    };
}