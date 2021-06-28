#include "common/common.h"
#include "common/Exception.h"
#include "common/FileOps.h"
#include "common/sort.h"
#include "os/os.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <array>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cxxabi.h>
#include <dirent.h>
#include <exception>
#include <memory>
#include <vector>

#include <sys/stat.h>

using namespace std;

namespace {
shared_ptr<spdlog::logger> makeFatalLogger() {
    auto alreadyExists = spdlog::get("fatalFallback");
    if (!alreadyExists) {
        return spdlog::stdout_color_mt("fatalFallback");
    }
    return alreadyExists;
}
} // namespace
shared_ptr<spdlog::logger> sorbet::fatalLogger = makeFatalLogger();

bool sorbet::FileOps::exists(string_view filename) {
    struct stat buffer;
    return (stat((string(filename)).c_str(), &buffer) == 0);
}

string sorbet::FileOps::read(string_view filename) {
    FILE *fp = std::fopen((string(filename)).c_str(), "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        auto sz = ftell(fp);
        string contents(sz, '\0');
        rewind(fp);
        auto readBytes = fread(&contents[0], 1, sz, fp);
        fclose(fp);
        if (readBytes != contents.size()) {
            // Error reading file?
            throw sorbet::FileNotFoundException();
        }
        return contents;
    }
    throw sorbet::FileNotFoundException();
}

void sorbet::FileOps::write(string_view filename, const vector<sorbet::u1> &data) {
    FILE *fp = std::fopen(string(filename).c_str(), "wb");
    if (fp) {
        fwrite(data.data(), sizeof(sorbet::u1), data.size(), fp);
        fclose(fp);
        return;
    }
    throw sorbet::FileNotFoundException();
}

bool sorbet::FileOps::dirExists(string_view path) {
    struct stat buffer;
    return stat((string(path)).c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode);
}

void sorbet::FileOps::createDir(string_view path) {
    auto err = mkdir(string(path).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (err) {
        throw sorbet::CreateDirException(fmt::format("Error in createDir('{}'): {}", path, errno));
    }
}

bool sorbet::FileOps::ensureDir(string_view path) {
    auto err = mkdir(string(path).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (err) {
        if (errno == EEXIST) {
            return false;
        }

        throw sorbet::CreateDirException(fmt::format("Error in createDir('{}'): {}", path, errno));
    }

    return true;
}

void sorbet::FileOps::removeDir(string_view path) {
    auto err = rmdir(string(path).c_str());
    if (err) {
        throw sorbet::CreateDirException(fmt::format("Error in removeDir('{}'): {}", path, errno));
    }
}

void sorbet::FileOps::removeFile(string_view path) {
    auto err = remove(string(path).c_str());
    if (err) {
        throw sorbet::RemoveFileException(fmt::format("Error in removeFile('{}'): {}", path, errno));
    }
}

void sorbet::FileOps::write(string_view filename, string_view text) {
    FILE *fp = std::fopen(string(filename).c_str(), "w");
    if (fp) {
        fwrite(text.data(), sizeof(char), text.size(), fp);
        fclose(fp);
        return;
    }
    throw sorbet::FileNotFoundException();
}

bool sorbet::FileOps::writeIfDifferent(string_view filename, string_view text) {
    if (!exists(filename) || text != read(filename)) {
        write(filename, text);
        return true;
    }
    return false;
}

void sorbet::FileOps::append(string_view filename, string_view text) {
    FILE *fp = std::fopen(string(filename).c_str(), "a");
    if (fp) {
        fwrite(text.data(), sizeof(char), text.size(), fp);
        fclose(fp);
        return;
    }
    throw sorbet::FileNotFoundException();
}

string_view sorbet::FileOps::getFileName(string_view path) {
    size_t found = path.find_last_of("/\\");
    return path.substr(found + 1);
}

string_view sorbet::FileOps::getExtension(string_view path) {
    size_t found = path.find_last_of(".");
    if (found == string_view::npos) {
        return string_view();
    }
    return path.substr(found + 1);
}

int sorbet::FileOps::readFd(int fd, std::vector<char> &output, int timeoutMs) {
    // Prepare to use select()
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);

    struct timeval timeout;
    // ms => seconds part
    timeout.tv_sec = timeoutMs / 1000;
    // ms => left over ms => converted to microseconds
    timeout.tv_usec = (timeoutMs % 1000) * 1000;

    auto rv = select(fd + 1, &set, nullptr, nullptr, &timeout);
    if (rv <= 0) {
        // A timeout (0) or error (<0) occurred
        return rv;
    }

    auto read = ::read(fd, output.data(), output.size());
    if (read <= 0) {
        // An error occurred.
        return -2;
    }
    // `read` is size read.
    return read;
}

sorbet::FileOps::ReadLineOutput sorbet::FileOps::readLineFromFd(int fd, string &buffer, int timeoutMs) {
    auto bufferFnd = buffer.find('\n');
    if (bufferFnd != string::npos) {
        // Edge case: Last time this was called, we read multiple lines.
        string line = buffer.substr(0, bufferFnd);
        buffer.erase(0, bufferFnd + 1);
        return ReadLineOutput{ReadResult::Success, line};
    }

    constexpr int BUFF_SIZE = 1024 * 8;
    vector<char> buf(BUFF_SIZE);

    int result = FileOps::readFd(fd, buf, timeoutMs);
    if (result == 0) {
        return ReadLineOutput{ReadResult::Timeout};
    } else if (result < 0) {
        return ReadLineOutput{ReadResult::ErrorOrEof};
    }

    // Store whatever we read into buffer, and see if we received a full line.
    const auto end = buf.begin() + result;
    const auto fnd = std::find(buf.begin(), end, '\n');
    if (fnd != end) {
        buffer.append(buf.begin(), fnd);
        string line = buffer;
        buffer.clear();
        if (fnd + 1 != end) {
            // If we read beyond the line, store extra stuff we read into the string buffer.
            // Skip over the newline.
            buffer.append(fnd + 1, end);
        }
        return ReadLineOutput{ReadResult::Success, line};
    } else {
        buffer.append(buf.begin(), end);
        return ReadLineOutput{ReadResult::Timeout};
    }
}

// Verifies that a matching pattern occurs at the end of the matched path
bool sorbet::FileOps::isFile(string_view path, string_view ignorePattern, const int pos) {
    const int endPos = pos + ignorePattern.length();
    return endPos == path.length();
}

// Verifies that a matching pattern is followed by a "/" in the matched path
bool sorbet::FileOps::isFolder(string_view path, string_view ignorePattern, const int pos) {
    const int endPos = pos + ignorePattern.length();
    return path.at(endPos) == '/';
}

// Simple, naive implementation of regexp-free ignore rules.
bool sorbet::FileOps::isFileIgnored(string_view basePath, string_view filePath,
                                    const vector<string> &absoluteIgnorePatterns,
                                    const vector<string> &relativeIgnorePatterns) {
    ENFORCE(filePath.substr(0, basePath.length()) == basePath);
    // Note: relative_path always includes a leading /
    string_view relative_path = filePath.substr(basePath.length());
    for (auto &p : absoluteIgnorePatterns) {
        if (relative_path.substr(0, p.length()) == p &&
            (isFile(relative_path, p, 0) || isFolder(relative_path, p, 0))) {
            return true;
        }
    }
    for (auto &p : relativeIgnorePatterns) {
        // See if /pattern is in string, and that it matches a whole folder or file name.
        int pos = 0;
        while (true) {
            pos = relative_path.find(p, pos);
            if (pos == string_view::npos) {
                break;
            } else if (isFile(relative_path, p, pos) || isFolder(relative_path, p, pos)) {
                return true;
            }
            pos += p.length();
        }
    }
    return false;
}

void appendFilesInDir(string_view basePath, const string &path, const sorbet::UnorderedSet<string> &extensions,
                      bool recursive, vector<string> &result, const std::vector<std::string> &absoluteIgnorePatterns,
                      const std::vector<std::string> &relativeIgnorePatterns) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path.c_str())) == nullptr) {
        switch (errno) {
            case ENOTDIR:
                throw sorbet::FileNotDirException();
            default:
                // Mirrors other FileOps functions: Assume other errors are from FileNotFound.
                throw sorbet::FileNotFoundException();
        }
    }

    while ((entry = readdir(dir)) != nullptr) {
        auto fullPath = fmt::format("{}/{}", path, entry->d_name);
        if (sorbet::FileOps::isFileIgnored(basePath, fullPath, absoluteIgnorePatterns, relativeIgnorePatterns)) {
            continue;
        } else if (entry->d_type == DT_DIR) {
            if (!recursive || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            appendFilesInDir(basePath, fullPath, extensions, recursive, result, absoluteIgnorePatterns,
                             relativeIgnorePatterns);
        } else {
            auto dotLocation = fullPath.rfind('.');
            if (dotLocation != string::npos) {
                string_view ext(fullPath.c_str() + dotLocation, fullPath.size() - dotLocation);
                if (extensions.contains(ext)) {
                    result.push_back(move(fullPath));
                }
            }
        }
    }
    closedir(dir);
}

vector<string> sorbet::FileOps::listFilesInDir(string_view path, const UnorderedSet<string> &extensions, bool recursive,
                                               const std::vector<std::string> &absoluteIgnorePatterns,
                                               const std::vector<std::string> &relativeIgnorePatterns) {
    vector<string> result;
    // Mini-optimization: appendFilesInDir needs to grab a c_str from path, so we pass in a string reference to avoid
    // copying.
    string pathStr(path);
    appendFilesInDir(path, pathStr, extensions, recursive, result, absoluteIgnorePatterns, relativeIgnorePatterns);
    fast_sort(result);
    return result;
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
sorbet::u4 sorbet::nextPowerOfTwo(sorbet::u4 v) {
    // Avoid underflow in subtraction on next line.
    if (v == 0) {
        // 1 is the nearest power of 2 to 0 (2^0)
        return 1;
    }
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

class SetTerminateHandler {
public:
    static void on_terminate() {
        sorbet::Exception::printBacktrace();
    }

    SetTerminateHandler() {
        set_terminate(&SetTerminateHandler::on_terminate);
    }
} SetTerminateHandler;

string exec(string cmd) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }
    while (feof(pipe.get()) == 0) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }
    return result;
}

string demangle(const char *mangled) {
    int status;
    unique_ptr<char[], void (*)(void *)> result(abi::__cxa_demangle(mangled, nullptr, nullptr, &status), free);
    return result.get() != nullptr ? string(result.get()) : "error occurred";
}
