#ifndef COMMON_FILESYSTEM_H
#define COMMON_FILESYSTEM_H

#include <string>
#include <vector>

namespace sorbet {

/**
 * File system interface.
 */
class FileSystem {
public:
    FileSystem() = default;
    virtual ~FileSystem() = default;

    /** Read the file at the given path. Throws a `FileNotFoundException` if not found. */
    virtual std::string readFile(std::string_view path) const = 0;

    /** Writes the specified data to the given file. */
    virtual void writeFile(std::string_view filename, std::string_view text) = 0;

    /** Returns a list of all files in the given directory. */
    virtual std::vector<std::string> listFilesInDir(std::string_view path, bool recursive) const = 0;
};

/**
 * An implementation of FileSystem that queries the OS's file system.
 */
class OSFileSystem final : public FileSystem {
public:
    OSFileSystem() = default;

    std::string readFile(std::string_view path) const override;
    void writeFile(std::string_view filename, std::string_view text) override;
    std::vector<std::string> listFilesInDir(std::string_view path, bool recursive) const override;
};

} // namespace sorbet

#endif // COMMON_FILESYSTEM_H