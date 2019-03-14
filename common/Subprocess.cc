#include "common/Subprocess.h"
#include "common/common.h"
#include <array>
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

// can't take string_views because we need char * not const char *
optional<string> sorbet::Subprocess::spawn(string executable, vector<string> arguments) {
    if (emscripten_build) {
        return nullopt;
    }
    int readWrite[2];
    int ret;
    pid_t childPid;

    ret = pipe(readWrite);
    if (ret) {
        return nullopt;
    }
    FileCloser closeRead(readWrite[0]);
    {
        FileCloser closeRead(readWrite[1]);

        FileActions fileActions;
        if (!fileActions.initialized()) {
            return nullopt;
        }

        // put the write end as stdout
        ret = posix_spawn_file_actions_adddup2(fileActions, readWrite[1], 1);
        if (ret) {
            return nullopt;
        }
        // close the read end of the pipe
        ret = posix_spawn_file_actions_addclose(fileActions, readWrite[0]);
        if (ret) {
            close(readWrite[1]);
            return nullopt;
        }

        vector<char *> argv;
        argv.reserve(arguments.size() + 2);
        argv.push_back(executable.data());
        for (auto &arg : arguments) {
            argv.push_back(arg.data());
        }
        argv.push_back(nullptr);

        ret = posix_spawnp(&childPid, executable.data(), fileActions, nullptr, argv.data(), nullptr);
        if (ret) {
            return nullopt;
        }
    }

    stringstream sink;
    array<char, 512> chunk;
    while (true) {
        const ssize_t bytesRead = read(readWrite[0], chunk.data(), chunk.size());
        if (bytesRead > 0) {
            sink << string_view(chunk.data(), bytesRead);
        } else if (bytesRead == 0) {
            break;
        } else /* bytesRead < 0 */ {
            waitpid(childPid, nullptr, 0); // reap the child so we don't have zombies
            return nullopt;
        }
    }

    int childStatus;
    pid_t waitResult = waitpid(childPid, &childStatus, 0);
    if (childPid != waitResult) {
        return nullopt;
    }
    if (childStatus != 0) {
        return nullopt;
    }

    return sink.str();
}
