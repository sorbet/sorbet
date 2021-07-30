#include "common/Path.h"

namespace sorbet {
using namespace std;

constexpr char file_separator = '/';

/**
 * A simple naive implementation of Path for platform that do not have <filesystem> yet
 */

template <> BasePath<string>::BasePath(const string_view path) : internal_path(std::string(path)) {}

template <> BasePath<string> &BasePath<string>::replaceFilename(const string_view filename) {
    auto last_sep = internal_path.rfind(file_separator);
    if (last_sep == std::string::npos) {
        internal_path = filename;
    } else {
        internal_path.replace(last_sep + 1, internal_path.size() - last_sep - 1, filename);
    }

    return *this;
}

template <> BasePath<string> &BasePath<string>::replaceExtension(const string_view extension) {
    auto last_sep = internal_path.rfind(file_separator);
    last_sep = last_sep == string::npos ? static_cast<size_t>(0) : last_sep + 1;
    auto last_dot = internal_path.find('.', last_sep);
    if (last_dot == string::npos) {
        internal_path += extension;
    } else {
        internal_path.replace(last_dot, internal_path.size() - last_dot, extension);
    }

    return *this;
}

template <> BasePath<string> &BasePath<string>::lexicallyNormal() {
    // TODO
    return *this;
}

template <> BasePath<string> &BasePath<string>::combineLeft(const string_view path) {
    internal_path.insert(0, &file_separator, 1);
    internal_path.insert(0, path);

    return *this;
}

template <> BasePath<string> &BasePath<string>::combineRight(const string_view path) {
    internal_path += file_separator;
    internal_path += path;

    return *this;
}

template <> const string BasePath<string>::string() {
    return internal_path;
}

#ifdef USE_STL_FILESYSTEM

/**
 * An more specialized implementation that leverages C++'s standard
 * <filesystem> library which is not available on emscripten yet
 */

template <> BasePath<filesystem::path>::BasePath(const string_view path) : internal_path(filesystem::path(path)) {}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::replaceFilename(const string_view filename) {
    internal_path.replace_filename(filename);
    return *this;
}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::replaceExtension(const string_view extension) {
    internal_path.replace_extension(extension);
    return *this;
}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::lexicallyNormal() {
    internal_path.lexically_normal();
    return *this;
}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::combineLeft(const string_view path) {
    internal_path = path / internal_path;
    return *this;
}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::combineRight(const string_view path) {
    internal_path /= path;
    return *this;
}

template <> const string BasePath<filesystem::path>::string() {
    return internal_path.string();
}

#endif

Path make_path(std::string_view path) {
    return Path(path);
}

} // namespace sorbet
