#include "common/common.h"
#include "absl/strings/escaping.h"
#include "common/FileOps.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"
#include "common/exception/Exception.h"
#include "common/sort/sort.h"
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
#include <variant>
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

bool sorbet::FileOps::exists(const string &filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

string sorbet::FileOps::read(const string &filename) {
    FILE *fp = std::fopen(filename.c_str(), "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        auto sz = ftell(fp);
        string contents(sz, '\0');
        rewind(fp);
        auto readBytes = fread(&contents[0], 1, sz, fp);
        fclose(fp);
        if (readBytes != contents.size()) {
            // Error reading file?
            auto msg = fmt::format("Error reading file: `{}`: {}", filename, errno);
            throw sorbet::FileNotFoundException(msg);
        }
        return contents;
    }
    auto msg = fmt::format("Cannot open file `{}`", filename);
    throw sorbet::FileNotFoundException(msg);
}

void sorbet::FileOps::write(const string &filename, const vector<uint8_t> &data) {
    FILE *fp = std::fopen(filename.c_str(), "wb");
    if (fp) {
        fwrite(data.data(), sizeof(uint8_t), data.size(), fp);
        fclose(fp);
        return;
    }
    auto msg = fmt::format("Cannot open file `{}` for writing", filename);
    throw sorbet::FileNotFoundException(msg);
}

bool sorbet::FileOps::dirExists(const string &path) {
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode);
}

void sorbet::FileOps::createDir(const string &path) {
    auto err = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (err) {
        auto msg = fmt::format("Error in createDir('{}'): {}", path, errno);
        throw sorbet::CreateDirException(msg);
    }
}

bool sorbet::FileOps::ensureDir(const string &path) {
    auto err = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (err) {
        if (errno == EEXIST) {
            return false;
        }

        auto msg = fmt::format("Error in createDir('{}'): {}", path, errno);
        throw sorbet::CreateDirException(msg);
    }

    return true;
}

void sorbet::FileOps::removeDir(const string &path) {
    auto err = rmdir(path.c_str());
    if (err) {
        auto msg = fmt::format("Error in removeDir('{}'): {}", path, errno);
        throw sorbet::RemoveDirException(msg);
    }
}

optional<string> sorbet::FileOps::realpath(const string &path) {
    unique_ptr<char[], void (*)(void *)> resolved(::realpath(path.c_str(), nullptr), free);
    if (resolved == nullptr) {
        return nullopt;
    }
    return string(resolved.get());
}

void sorbet::FileOps::removeFile(const string &path) {
    auto err = remove(path.c_str());
    if (err) {
        auto msg = fmt::format("Error in removeFile('{}'): {}", path, errno);
        throw sorbet::RemoveFileException(msg);
    }
}

void sorbet::FileOps::write(const string &filename, string_view text) {
    FILE *fp = std::fopen(filename.c_str(), "w");
    if (fp) {
        fwrite(text.data(), sizeof(char), text.size(), fp);
        fclose(fp);
        return;
    }
    auto msg = fmt::format("Cannot open file `{}` for writing", filename);
    throw sorbet::FileNotFoundException(msg);
}

bool sorbet::FileOps::writeIfDifferent(const string &filename, string_view text) {
    if (!exists(filename) || text != read(filename)) {
        write(filename, text);
        return true;
    }
    return false;
}

void sorbet::FileOps::append(const string &filename, string_view text) {
    FILE *fp = std::fopen(filename.c_str(), "a");
    if (fp) {
        fwrite(text.data(), sizeof(char), text.size(), fp);
        fclose(fp);
        return;
    }
    auto msg = fmt::format("Cannot open file `{}` for writing", filename);
    throw sorbet::FileNotFoundException(msg);
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

bool sorbet::FileOps::hasAllowedExtension(string_view path, const UnorderedSet<string> &extensions) {
    auto dotLocation = path.rfind('.');
    if (dotLocation == string_view::npos) {
        return false;
    }

    string_view ext = path.substr(dotLocation);
    return extensions.contains(ext);
}

int sorbet::FileOps::readFd(int fd, absl::Span<char> output, int timeoutMs) {
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
        return ReadLineOutput{ReadResult::Success, std::move(line)};
    }

    constexpr int BUFF_SIZE = 1024 * 8;
    char chars[BUFF_SIZE];
    absl::Span<char> buf(chars);

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
        string line = std::move(buffer);
        buffer.clear();
        if (fnd + 1 != end) {
            // If we read beyond the line, store extra stuff we read into the string buffer.
            // Skip over the newline.
            buffer.append(fnd + 1, end);
        }
        return ReadLineOutput{ReadResult::Success, std::move(line)};
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

struct QuitToken {};
using Job = variant<QuitToken, string>;
// We record all classes of exceptions we can throw separately, rather than one single `SorbetException`.
//
// We do this for two reasons: one is that when looking for a `catch` handler, C++ will only look for
// a handler matching the *static* type of the thrown exception, so when we rethrow the exceptions from
// the workers on the main thread, we need the `throw` expressions to have the proper static types.
//
// "Why not use `dynamic_cast`?" you might ask: store a `SorbetException` here and then downcast as
// necessary when rethrowing the exception.  The problem with that idea is that the originally thrown
// exception would get copied into a `JobOutput` object as a `SorbetException`, and therefore the
// dynamic type of the original exception would be lost.
using JobOutput = variant<std::monostate, sorbet::FileNotFoundException, sorbet::FileNotDirException, vector<string>>;

void appendFilesInDir(string_view basePath, const string &path, const sorbet::UnorderedSet<string> &extensions,
                      sorbet::WorkerPool &workers, bool recursive, vector<string> &allPaths,
                      const vector<string> &absoluteIgnorePatterns, const vector<string> &relativeIgnorePatterns) {
    auto numWorkers = max(workers.size(), 1);
    auto jobq = make_shared<ConcurrentUnBoundedQueue<Job>>();
    auto resultq = make_shared<BlockingBoundedQueue<JobOutput>>(numWorkers);
    atomic<size_t> pendingJobs{0};

    // The invariant that the code below must maintain is pendingJobs must be
    // at least as large as the number of items in jobq.  Therefore, once
    // pendingJobs is 0, whatever thread observes that can be assured that there
    // is no more work to be done and can initiate shutdown.
    //
    // In practice, all it takes to maintain this invariant is that pendingJobs
    // must be incremented prior to pushing work onto jobq.
    ++pendingJobs;
    jobq->push(path, 1);

    workers.multiplexJob("options.findFiles", [numWorkers, jobq, resultq, &pendingJobs, &basePath, &extensions,
                                               &recursive, &absoluteIgnorePatterns, &relativeIgnorePatterns]() {
        Job job;
        vector<string> output;

        try {
            while (true) {
                auto result = jobq->try_pop(job);
                if (!result.gotItem()) {
                    continue;
                }
                if (std::holds_alternative<QuitToken>(job)) {
                    break;
                }

                auto *strvariant = std::get_if<string>(&job);
                ENFORCE(strvariant != nullptr);
                auto &path = *strvariant;

                DIR *dir;
                struct dirent *entry;

                if ((dir = opendir(path.c_str())) == nullptr) {
                    switch (errno) {
                        case ENOTDIR: {
                            throw sorbet::FileNotDirException();
                        }
                        default:
                            // Mirrors other FileOps functions: Assume other errors are from FileNotFound.
                            auto msg = fmt::format("Couldn't open directory `{}`", path);
                            throw sorbet::FileNotFoundException(msg);
                    }
                }

                while ((entry = readdir(dir)) != nullptr) {
                    const auto namelen = strlen(entry->d_name);
                    string_view nameview{entry->d_name, namelen};
                    if (entry->d_type == DT_DIR) {
                        if (!recursive) {
                            continue;
                        }
                        if (nameview == "."sv || nameview == ".."sv) {
                            continue;
                        }
                    } else {
                        if (!sorbet::FileOps::hasAllowedExtension(nameview, extensions)) {
                            continue;
                        }
                    }

                    auto fullPath = fmt::format("{}/{}", path, nameview);
                    if (sorbet::FileOps::isFileIgnored(basePath, fullPath, absoluteIgnorePatterns,
                                                       relativeIgnorePatterns)) {
                        continue;
                    }

                    if (entry->d_type == DT_DIR) {
                        ++pendingJobs;
                        jobq->push(move(fullPath), 1);
                    } else {
                        output.push_back(move(fullPath));
                    }
                }

                closedir(dir);

                // Now that we've finished with this directory, we can decrement.
                auto remaining = --pendingJobs;
                // If this thread is finished with the last job in the queue, then
                // we can start signaling other threads that they need to quit.
                if (remaining == 0) {
                    // Maintain the invariant, even though we're all done.
                    pendingJobs += numWorkers;
                    for (auto i = 0; i < numWorkers; ++i) {
                        jobq->push(QuitToken{}, 1);
                    }
                    break;
                }
            }
        } catch (sorbet::FileNotFoundException &e) {
            resultq->push(e, 1);
            pendingJobs += numWorkers;
            for (auto i = 0; i < numWorkers; ++i) {
                jobq->push(QuitToken{}, 1);
            }
            return;
        } catch (sorbet::FileNotDirException &e) {
            resultq->push(e, 1);
            pendingJobs += numWorkers;
            for (auto i = 0; i < numWorkers; ++i) {
                jobq->push(QuitToken{}, 1);
            }
            return;
        }

        resultq->push(move(output), 1);
    });

    {
        JobOutput threadResult;
        optional<sorbet::FileNotFoundException> fileNotFound;
        optional<sorbet::FileNotDirException> fileNotDir;
        auto &logger = *spdlog::default_logger();
        for (auto result = resultq->wait_pop_timed(threadResult, sorbet::WorkerPool::BLOCK_INTERVAL(), logger);
             !result.done();
             result = resultq->wait_pop_timed(threadResult, sorbet::WorkerPool::BLOCK_INTERVAL(), logger)) {
            if (result.gotItem()) {
                if (auto *paths = std::get_if<vector<string>>(&threadResult)) {
                    allPaths.insert(allPaths.end(), make_move_iterator(paths->begin()),
                                    make_move_iterator(paths->end()));
                    continue;
                }

                if (auto *e = std::get_if<sorbet::FileNotFoundException>(&threadResult)) {
                    if (!fileNotFound.has_value()) {
                        fileNotFound = *e;
                    }
                } else if (auto *e = std::get_if<sorbet::FileNotDirException>(&threadResult)) {
                    if (!fileNotDir.has_value()) {
                        fileNotDir = *e;
                    }
                } else {
                    ENFORCE(false, "should never get here!");
                }
            }
        }

        // If there was an error, don't raise it until after all the worker threads have finished,
        // because they might be working on something, attempting to write to the pendingJobs
        // variable stored on our stack, and then suddenly see that stack address gone because we've
        // raised and jumped elsewhere.
        if (fileNotFound.has_value()) {
            throw fileNotFound.value();
        } else if (fileNotDir.has_value()) {
            throw fileNotDir.value();
        }
    }
}

vector<string> sorbet::FileOps::listFilesInDir(string_view path, const UnorderedSet<string> &extensions,
                                               WorkerPool &workerPool, bool recursive,
                                               const vector<string> &absoluteIgnorePatterns,
                                               const vector<string> &relativeIgnorePatterns) {
    vector<string> result;
    // Mini-optimization: appendFilesInDir needs to grab a c_str from path, so we pass in a string reference to avoid
    // copying.
    string pathStr(path);
    appendFilesInDir(path, pathStr, extensions, workerPool, recursive, result, absoluteIgnorePatterns,
                     relativeIgnorePatterns);
    fast_sort(result);
    return result;
}

vector<string> sorbet::FileOps::listSubdirs(const string &path) {
    vector<string> result;

    DIR *dir;
    if ((dir = opendir(path.c_str())) == nullptr) {
        switch (errno) {
            case ENOTDIR:
                throw sorbet::FileNotDirException();

            default:
                // Mirrors other FileOps functions: Assume other errors are from FileNotFound.
                auto msg = fmt::format("Couldn't open directory `{}`", path);
                throw sorbet::FileNotFoundException(msg);
        }
    }

    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            auto namelen = strlen(entry->d_name);
            std::string_view name{entry->d_name, namelen};
            if (name == "." || name == "..") {
                continue;
            }

            result.emplace_back(name);
        }
    }

    closedir(dir);
    return result;
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
uint32_t sorbet::nextPowerOfTwo(uint32_t v) {
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

sorbet::ProcessStatus sorbet::processExists(pid_t pid) {
    if (kill(pid, 0) == 0) {
        return ProcessStatus::Running;
    }

    switch (errno) {
        case EINVAL:
            // This should be impossible, as we're using the signal value of `0`, which is well-defined.
            return ProcessStatus::Unknown;

        case EPERM:
            // The process does exist, but we don't have permissions to send it signals.
            return ProcessStatus::Running;

        case ESRCH:
            // No process exists with this pid.
            return ProcessStatus::Missing;

        default:
            // We return `Unknown` here as the above three cases are the only ones called out in the man page for the
            // kill syscall.
            return ProcessStatus::Unknown;
    }
}

vector<uint32_t> sorbet::findLineBreaks(string_view s) {
    vector<uint32_t> res;
    size_t next_pos = 0;
    while (true) {
        auto pos = s.find_first_of('\n', next_pos);
        if (pos == string_view::npos) {
            break;
        }

        res.emplace_back((int)pos);
        next_pos = pos + 1;
    }

    res.emplace_back(s.size());
    return res;
}

class SetTerminateHandler {
public:
    static void on_terminate() {
        auto eptr = current_exception();
        if (eptr) {
            try {
                // Apparently this is the only way to convert a std::exception_ptr to std::exception
                std::rethrow_exception(eptr);
            } catch (const std::exception &e) {
                sorbet::fatalLogger->error("Sorbet raised uncaught exception type=\"{}\" what=\"{}\"",
                                           demangle(typeid(e).name()), absl::CEscape(e.what()));
            } catch (const string &s) {
                sorbet::fatalLogger->error("Sorbet raised uncaught exception type=string what=\"{}\"",
                                           absl::CEscape(s));
            } catch (const char *s) {
                sorbet::fatalLogger->error("Sorbet raised uncaught exception type=\"char *\" what=\"{}\"",
                                           absl::CEscape(s));
            } catch (...) {
                sorbet::fatalLogger->error("Sorbet raised uncaught exception type=<unknown> what=\"\"");
            }
        } else {
            sorbet::fatalLogger->error("Sorbet raised uncaught exception type=<unknown> what=\"\"");
        }

        // Might print the backtrace twice if it's a SorbetException, because
        // Exception::raise already prints the backtrace when the exception is raised. But
        // SorbetException are caught at various places inside Sorbet, and so might not
        // always terminate the process.
        sorbet::Exception::printBacktrace();

        // "A std::terminate_handler shall terminate execution of the program without returning to
        // the caller, otherwise the behavior is undefined."
        //
        // In libc++, the behavior is to print "terminate_handler unexpectedly returned" and then
        // call abort()
        abort();
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
