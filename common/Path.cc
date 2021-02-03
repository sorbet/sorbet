#include "common/Path.h"

namespace sorbet {
using namespace std;

#if defined(_WIN32) || defined(_WIN64)
constexpr char file_separator = '\\';
#else
constexpr char file_separator = '/';
#endif

/**
 * A simple naive implementation of Path for platform that do not have <filesystem> yet
 */

template <> BasePath<string>::BasePath(const string_view path) : internal_path(std::string(path)) {}

template <> BasePath<string> &BasePath<string>::replace_filename(const string_view filename) {
    auto last_sep = internal_path.rfind(file_separator);
    if (last_sep == std::string::npos)
        internal_path = filename;
    else
        internal_path.replace(last_sep + 1, internal_path.size() - last_sep - 1, filename);

    return *this;
}

template <> BasePath<string> &BasePath<string>::replace_extension(const string_view extension) {
    auto last_sep = internal_path.rfind(file_separator);
    last_sep = last_sep == string::npos ? static_cast<size_t>(-1) : last_sep;
    auto last_dot = internal_path.find('.', last_sep + 1);
    if (last_dot == string::npos)
        internal_path += extension;
    else
        internal_path.replace(last_dot, internal_path.size() - last_dot, extension);

    return *this;
}

template <> BasePath<string> &BasePath<string>::lexically_normal() {
    // TODO
    return *this;
}

template <> BasePath<string> &BasePath<string>::combine_left(const string_view path) {
    internal_path.insert(0, &file_separator, 1);
    internal_path.insert(0, path);

    return *this;
}

template <> BasePath<string> &BasePath<string>::combine_right(const string_view path) {
    internal_path += file_separator;
    internal_path += path;

    return *this;
}

template <> const string_view BasePath<string>::string() {
    return internal_path;
}

#ifdef USE_STL_FILESYSTEM

/**
 * An more specialized implementation that leverages C++'s standard
 * <filesystem> library which is not available on emscripten yet
 */

template <> BasePath<filesystem::path>::BasePath(const string_view path) : internal_path(filesystem::path(path)) {}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::replace_filename(const string_view filename) {
    internal_path.replace_filename(filename);
    return *this;
}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::replace_extension(const string_view extension) {
    internal_path.replace_extension(extension);
    return *this;
}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::lexically_normal() {
    internal_path.lexically_normal();
    return *this;
}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::combine_left(const string_view path) {
    internal_path = path / internal_path;
    return *this;
}

template <> BasePath<filesystem::path> &BasePath<filesystem::path>::combine_right(const string_view path) {
    internal_path /= path;
    return *this;
}

template <> const string_view BasePath<filesystem::path>::string() {
    return internal_path.string();
}

#endif

Path make_path(std::string_view path) {
    return Path(path);
}

} // namespace sorbet
