#include "PathHelper.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace Flux {

    std::string PathHelper::GetAssetPath(const std::string& relativePath) {
        std::filesystem::path basePath;

#ifdef _WIN32
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        basePath = std::filesystem::path(buffer).parent_path();

        if (std::filesystem::exists(basePath / "assets")) {
            return (basePath / relativePath).string();
        }
#else
        char buffer[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
        if (count != -1) {
            basePath = std::filesystem::path(std::string(buffer, count)).parent_path();
        } else {
            basePath = std::filesystem::current_path();
        }

        if (std::filesystem::exists(basePath / "assets")) {
            return (basePath / relativePath).string();
        }

        std::filesystem::path linuxSharePath = basePath.parent_path() / "share" / "flux";
        if (std::filesystem::exists(linuxSharePath / "assets")) {
            return (linuxSharePath / relativePath).string();
        }
#endif

        return relativePath;
    }

}