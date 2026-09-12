#include "shiplua/manifest/ModDiscovery.h"
#include "shiplua/manifest/PackageFormat.h"

#include <algorithm>
#include <string>
#include <miniz.h>

namespace ShipLua {

namespace {

bool IsRegularFileWithoutSymlink(const std::filesystem::path& path, std::error_code& error) {
    const auto status = std::filesystem::symlink_status(path, error);
    if (error == std::errc::no_such_file_or_directory) {
        error.clear();
        return false;
    }
    return !error && std::filesystem::is_regular_file(status) &&
           !std::filesystem::is_symlink(status);
}

bool HasRootManifest(const std::filesystem::path& path) {
    mz_zip_archive archive{};
    if (!mz_zip_reader_init_file(&archive, path.string().c_str(), 0)) {
        return false;
    }
    const int index = mz_zip_reader_locate_file(
        &archive, "manifest.toml", nullptr, MZ_ZIP_FLAG_CASE_SENSITIVE);
    mz_zip_archive_file_stat stat{};
    const bool found = index >= 0 &&
        mz_zip_reader_file_stat(&archive, static_cast<mz_uint>(index), &stat) &&
        !stat.m_is_directory && ((stat.m_external_attr >> 16U) & 0170000U) != 0120000U;
    mz_zip_reader_end(&archive);
    return found;
}

} // namespace

bool HasModPackageManifest(const std::filesystem::path& path) {
    std::error_code error;
    return IsRegularFileWithoutSymlink(path, error) && HasRootManifest(path);
}

Result<std::vector<DiscoveredMod>> DiscoverMods(const std::filesystem::path& root) {
    std::error_code error;
    const auto rootStatus = std::filesystem::symlink_status(root, error);
    if (error || !std::filesystem::is_directory(rootStatus) ||
        std::filesystem::is_symlink(rootStatus)) {
        return Result<std::vector<DiscoveredMod>>::err(
            ErrorCode::InvalidArgument, "mods root is not a readable directory: '" + root.string() + "'");
    }

    std::vector<DiscoveredMod> mods;
    std::filesystem::directory_iterator iterator(
        root, std::filesystem::directory_options::skip_permission_denied, error);
    const std::filesystem::directory_iterator end;
    if (error) {
        return Result<std::vector<DiscoveredMod>>::err(
            ErrorCode::InvalidArgument, "cannot scan mods root '" + root.string() + "': " + error.message());
    }

    while (iterator != end) {
        const std::filesystem::directory_entry entry = *iterator;
        const auto status = entry.symlink_status(error);
        if (error) {
            return Result<std::vector<DiscoveredMod>>::err(
                ErrorCode::InvalidArgument, "cannot inspect mod candidate '" +
                                                entry.path().string() + "': " + error.message());
        }

        if (!std::filesystem::is_symlink(status) && std::filesystem::is_directory(status)) {
            const auto manifestPath = entry.path() / "manifest.toml";
            if (IsRegularFileWithoutSymlink(manifestPath, error)) {
                mods.push_back({entry.path(), ModSourceKind::Directory});
            } else if (error) {
                return Result<std::vector<DiscoveredMod>>::err(
                    ErrorCode::InvalidArgument, "cannot inspect manifest '" +
                                                    manifestPath.string() + "': " + error.message());
            }
        } else if (!std::filesystem::is_symlink(status) &&
                   std::filesystem::is_regular_file(status) &&
                   HasModPackageExtension(entry.path()) && HasRootManifest(entry.path())) {
            mods.push_back({entry.path(), ModSourceKind::Package});
        }

        iterator.increment(error);
        if (error) {
            return Result<std::vector<DiscoveredMod>>::err(
                ErrorCode::InvalidArgument, "cannot continue scanning mods root '" +
                                                root.string() + "': " + error.message());
        }
    }

    std::sort(mods.begin(), mods.end(), [](const DiscoveredMod& left, const DiscoveredMod& right) {
        return left.path.generic_string() < right.path.generic_string();
    });
    return Result<std::vector<DiscoveredMod>>::ok(std::move(mods));
}

} // namespace ShipLua
