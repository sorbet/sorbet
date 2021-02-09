#ifndef COMMON_PATH_H
#define COMMON_PATH_H

#include "common/common.h"
#include <string>
#include <vector>

#ifdef USE_STL_FILESYSTEM
#include <filesystem>
#endif

namespace sorbet {

/**
 * Path interface.
 * Note that this base implementation is not functional
 * rather you are meant to use one of the specialization
 * (either for std::filesystem::path or raw std::string)
 * which is conveniently selected for you by the "Path"
 * typedef.
 */
template <typename TInternalPath> class BasePath {
public:
    BasePath(std::string_view path) {}

    /**
     * Replace the filename portion of the path or append it.
     * Returns self
     */
    BasePath<TInternalPath> &replaceFilename(const std::string_view filename) {
        return *this;
    }
    /**
     * Replace the filename extension in the path, if there was no extension append it.
     * Returns self
     */
    BasePath<TInternalPath> &replaceExtension(const std::string_view extension) {
        return *this;
    }
    /**
     * Replace the filename portion of the path. Returns self
     */
    BasePath<TInternalPath> &lexicallyNormal() {
        return *this;
    }
    /**
     * Replace the filename portion of the path. Returns self
     */
    BasePath<TInternalPath> &combineLeft(const std::string_view path) {
        return *this;
    }
    /**
     * Replace the filename portion of the path. Returns self
     */
    BasePath<TInternalPath> &combineRight(const std::string_view path) {
        return *this;
    }
    /**
     * Returns the string representation of the path
     */
    const std::string string() {
        return "";
    }

protected:
    TInternalPath internal_path;
};

#ifdef USE_STL_FILESYSTEM
typedef BasePath<std::filesystem::path> Path;
#else
typedef BasePath<std::string> Path;
#endif

Path make_path(std::string_view path);

} // namespace sorbet

#endif // COMMON_PATH_H