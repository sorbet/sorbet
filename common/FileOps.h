#ifndef SORBET_COMMON_FILEOPS_HPP
#define SORBET_COMMON_FILEOPS_HPP
#include "absl/types/span.h"
#include "common/common.h"
#include <string>
#include <string_view>
#include <vector>

namespace sorbet {
class WorkerPool;

class FileOps final {
public:
    enum class ReadResult { Timeout, Success, ErrorOrEof };
    struct ReadLineOutput {
        ReadResult result;
        std::optional<std::string> output = std::nullopt;
    };

    static bool exists(const std::string &filename);
    static bool isFile(std::string_view path, std::string_view ignorePattern, const int pos);
    static bool isFolder(std::string_view path, std::string_view ignorePattern, const int pos);
    static std::string read(const std::string &filename);
    static void write(const std::string &filename, const std::vector<uint8_t> &data);
    static void append(const std::string &filename, std::string_view text);
    static void write(const std::string &filename, std::string_view text);
    static bool writeIfDifferent(const std::string &filename, std::string_view text);
    static bool dirExists(const std::string &path);
    static void createDir(const std::string &path);
    static std::optional<std::string> realpath(const std::string &path);

    // This differs from createDir, as it will not raise an exception if the directory already exists. Returns true when
    // the directory was created, and false if it already existed.
    //
    // NOTE: This does not create parent directories if they exist.
    static bool ensureDir(const std::string &path);

    // NOTE: this is a minimal wrapper around rmdir, and as such will raise an exception if the directory is not empty
    // when it's removed.
    static void removeDir(const std::string &path);

    static void removeFile(const std::string &path);

    /**
     * Returns a list of all files in the given directory. Returns paths that include the path to directory.
     * Throws FileNotFoundException if path does not exist, and FileNotDirException if path is not a directory.
     */
    static std::vector<std::string> listFilesInDir(std::string_view path, const UnorderedSet<std::string> &extensions,
                                                   WorkerPool &workers, bool recursive,
                                                   const std::vector<std::string> &absoluteIgnorePatterns,
                                                   const std::vector<std::string> &relativeIgnorePatterns);

    /**
     * Lists out the subdirectories of the given directory.
     * Throws FileNotFoundException if path does not exist, and FileNotDirException if path is not a directory.
     */
    static std::vector<std::string> listSubdirs(const std::string &path);

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
     * Returns 'true' if the file at the given path has an extension that matches one of those in 'extensions'.
     */
    static bool hasAllowedExtension(std::string_view path, const UnorderedSet<std::string> &extensions);

    /**
     * Reads data from the given file descriptor, and stores it into the output buffer.
     * Reads up to `output.size()` bytes. Timeout is specified in milliseconds.
     * Returns:
     * - Length of data read if > 0.
     * - 0 if timeout occurs.
     * - <0 if an error or EOF occurs.
     */
    static int readFd(int fd, absl::Span<char> output, int timeoutMs = 100);

    /**
     * Attempts to read data up to a newline (\n) from the given file descriptor.
     * Timeout is specified in milliseconds.
     *
     * Stores any extra bits read into `buffer`.
     */
    static ReadLineOutput readLineFromFd(int fd, std::string &buffer, int timeoutMs = 100);
};

} // namespace sorbet

#endif
