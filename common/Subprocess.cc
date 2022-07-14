#include "common/Subprocess.h"
#include "common/common.h"
#include <array>
#include <fcntl.h>
#include <spawn.h>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;

namespace {
class FileCloser {
public:
    explicit FileCloser(int filedes) : filedes(filedes) {}

    ~FileCloser() {
        close(filedes);
    }

private:
    int filedes;
};

class FileActions {
public:
    FileActions() {
        if (posix_spawn_file_actions_init(&fileActions) == 0) {
            _initialized = true;
        } else {
            _initialized = false;
        }
    }

    operator posix_spawn_file_actions_t *() {
        return &fileActions;
    }

    bool initialized() {
        return _initialized;
    }

    ~FileActions() {
        if (_initialized) {
            posix_spawn_file_actions_destroy(&fileActions);
        }
    }

private:
    posix_spawn_file_actions_t fileActions;
    bool _initialized;
};
} // namespace

// Spawns a new child process, pipes `stdinContents` into the new processes's stdin,
// and returns the child's stdout and status code
optional<sorbet::Subprocess::Result> sorbet::Subprocess::spawn(string executable, vector<string> arguments,
                                                               optional<string_view> stdinContents) {
    if (emscripten_build) {
        return nullopt;
    }
    int stdinPipe[2];
    int stdoutPipe[2];
    int ret;

    pid_t childPid;

    // setup pipes
    ret = pipe(stdoutPipe);
    if (ret) {
        return nullopt;
    }
    ret = pipe(stdinPipe);
    if (ret) {
        return nullopt;
    }

    FileCloser closeStdoutRead(stdoutPipe[0]);
    FileCloser closeStdinRead(stdinPipe[0]);
    {
        FileCloser closeStdoutWrite(stdoutPipe[1]);
        FileCloser closeStdinWrite(stdinPipe[1]);

        FileActions fileActions;
        if (!fileActions.initialized()) {
            return nullopt;
        }

        // Put the child's write end as stdout
        ret = posix_spawn_file_actions_adddup2(fileActions, stdoutPipe[1], 1);
        if (ret) {
            return nullopt;
        }

        // Put the child's read end as stdin
        ret = posix_spawn_file_actions_adddup2(fileActions, stdinPipe[0], 0);
        if (ret) {
            return nullopt;
        }

        vector<char *> argv;
        argv.reserve(arguments.size() + 2);
        argv.push_back(executable.data());
        for (auto &arg : arguments) {
            argv.push_back(arg.data());
        }
        argv.push_back(nullptr);

        // Close child copy of the file descriptor on exec
        fcntl(stdinPipe[1], F_SETFD, FD_CLOEXEC);

        ret = posix_spawnp(&childPid, executable.data(), fileActions, nullptr, argv.data(), nullptr);
        if (ret) {
            return nullopt;
        }

        // Write contents to child process stdin
        if (stdinContents.has_value()) {
            ret = write(stdinPipe[1], stdinContents->data(), stdinContents->size());
            if (ret < 0) {
                return nullopt;
            }
        }

        // Close child process's stdin
        ret = posix_spawn_file_actions_addclose(fileActions, stdinPipe[1]);
        if (ret) {
            return nullopt;
        }
    }

    stringstream sink;
    array<char, 512> chunk;
    while (true) {
        const ssize_t bytesRead = read(stdoutPipe[0], chunk.data(), chunk.size());
        if (bytesRead > 0) {
            sink << string_view(chunk.data(), bytesRead);
        } else if (bytesRead == 0) {
            break;
        } else /* bytesRead < 0 */ {
            if (errno == EINTR) {
                continue; // this happens when we are being debugged
            }
            waitpid(childPid, nullptr, 0); // reap the child so we don't have zombies
            return nullopt;
        }
    }

    int childStatus;
    pid_t waitResult = waitpid(childPid, &childStatus, 0);
    if (childPid != waitResult) {
        return nullopt;
    }

    return sorbet::Subprocess::Result{sink.str(), WEXITSTATUS(childStatus)};
}
