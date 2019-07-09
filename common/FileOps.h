#ifndef SORBET_COMMON_FILEOPS_HPP
#define SORBET_COMMON_FILEOPS_HPP
#include "common/common.h"
#include <string>
#include <string_view>
#include <vector>

namespace sorbet {

class FileOps final {
public:
    static bool exists(std::string_view filename);
    static std::string read(std::string_view filename);
    static void write(std::string_view filename, const std::vector<sorbet::u1> &data);
    static void append(std::string_view filename, std::string_view text);
    static void write(std::string_view filename, std::string_view text);
    static bool writeIfDifferent(std::string_view filename, std::string_view text);
    static bool dirExists(std::string_view path);
    static void createDir(std::string_view path);
    static void removeFile(std::string_view path);
    /**
     * Returns a list of all files in the given directory. Returns paths that include the path to directory.
     * Throws FileNotFoundException if path does not exist, and FileNotDirException if path is not a directory.
     */
    static std::vector<std::string> listFilesInDir(std::string_view path, const UnorderedSet<std::string> &extensions,
                                                   bool recursive,
                                                   const std::vector<std::string> &absoluteIgnorePatterns,
                                                   const std::vector<std::string> &relativeIgnorePatterns);
    /**
     * Returns 'true' if the file at the given path is ignored.
     * See sorbet::options for information on absolute and relative ignore patterns.
     */
    static bool isFileIgnored(std::string_view basePath, std::string_view filePath,
                              const std::vector<std::string> &absoluteIgnorePatterns,
                              const std::vector<std::string> &relativeIgnorePatterns);
    static std::string_view getFileName(std::string_view path);
    static std::string_view getExtension(std::string_view path);
    /**
     * Reads data from the given file descriptor, and stores it into the output buffer.
     * Reads up to `output.size()` bytes. Timeout is specified in milliseconds.
     * Returns:
     * - Length of data read if > 0.
     * - 0 if timeout occurs.
     *
     * Throws FileReadException on EOF or error.
     */
    static int readFd(int fd, std::vector<char> &output, int timeoutMs = 100);
    /**
     * Attempts to read data up to a newline (\n) from the given file descriptor.
     * Timeout is specified in milliseconds.
     *
     * Stores any extra bits read into `buffer`.
     *
     * Returns:
     * - nullopt if timeout occurs or read() did not yield a newline.
     * - string if a line was found. The string does not contain the ending newline character.
     *
     * Throws FileReadException on EOF or error.
     */
    static std::optional<std::string> readLineFromFd(int fd, std::string &buffer, int timeoutMs = 100);
};

} // namespace sorbet

#endif
