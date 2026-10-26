#include "shiplua/host/ModPath.h"

#include <utility>

namespace ShipLua {

Result<std::filesystem::path> ResolveModFile(const std::filesystem::path& modDir,
                                                const std::string& relativePath) {
    const std::filesystem::path relative(relativePath);
    if (relative.empty() || relative.is_absolute() || relative.has_root_name() ||
        relative.has_root_directory()) {
        return Result<std::filesystem::path>::err(
            ErrorCode::PermissionDenied, "entrypoint must be a relative path inside the mod directory");
    }

    const std::filesystem::path normalized = relative.lexically_normal();
    for (const auto& component : normalized) {
        if (component == "..") {
            return Result<std::filesystem::path>::err(
                ErrorCode::PermissionDenied, "entrypoint cannot contain parent traversal ('..')");
        }
    }

    std::error_code error;
    std::filesystem::path current = std::filesystem::weakly_canonical(modDir, error);
    if (error) {
        return Result<std::filesystem::path>::err(
            ErrorCode::InvalidArgument, "cannot resolve mod directory: " + error.message());
    }
    for (const auto& component : normalized) {
        current /= component;
        const auto status = std::filesystem::symlink_status(current, error);
        if (error) {
            return Result<std::filesystem::path>::err(
                ErrorCode::InvalidArgument, "cannot inspect entrypoint path: " + error.message());
        }
        if (std::filesystem::is_symlink(status)) {
            return Result<std::filesystem::path>::err(
                ErrorCode::PermissionDenied, "entrypoint path cannot contain symbolic links");
        }
    }
    return Result<std::filesystem::path>::ok(std::move(current));
}

} // namespace ShipLua
