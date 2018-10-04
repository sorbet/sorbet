#ifdef __linux__
#include "absl/debugging/symbolize.h"
#include "common/common.h"
#include <climits>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

extern "C" {
// If we are linked against the LLVM sanitizers, this symbol will be
// replaced with the definition from the sanitizer runtime
void __attribute__((weak)) __sanitizer_symbolize_pc(void *pc, const char *fmt, char *out_buf, size_t out_buf_size) {
    snprintf(out_buf, out_buf_size, "<null>");
}
}

string exec(string cmd);

static void symbolize_pc(void *pc, const char *fmt, char *out_buf, size_t out_buf_size) {
    __sanitizer_symbolize_pc(pc, fmt, out_buf, out_buf_size);
    out_buf[out_buf_size - 1] = '\0';
    if (strstr(out_buf, "<null>") != nullptr) { // sanitizers were'nt able to symbolize it.
        auto offset = snprintf(out_buf, out_buf_size, "%p ", pc);
        absl::Symbolize(pc, out_buf + offset, out_buf_size - offset);
    }
}

string addr2line(const string program_name, void const *const *addr, int count) {
    stringstream os;
    for (int i = 3; i < count; ++i) {
        char buf[4096];
        symbolize_pc(const_cast<void *>(addr[i]), "%p in %f %s:%l:%c", buf, sizeof(buf));
        os << "  #" << i << " " << buf << "\n";
    }
    return os.str();
}
string getProgramName() {
    char dest[512] = {}; // explicitly zero out

    if (readlink("/proc/self/exe", dest, PATH_MAX) < 0) {
        return "<error>";
    }

    string res(dest);
    return res;
}

bool amIBeingDebugged() {
    // TracerPid was added into linux in ~2005. Should work on all linuxes since then
    char buf[4096];
    // cargo culted from
    // https://stackoverflow.com/questions/3596781/how-to-detect-if-the-current-process-is-being-run-by-gdb

    const int status_fd = ::open("/proc/self/status", O_RDONLY);
    if (status_fd == -1) {
        return false;
    }

    const ssize_t num_read = ::read(status_fd, buf, sizeof(buf) - 1);
    ::close(status_fd);
    if (num_read <= 0) {
        return false;
    }

    buf[num_read] = '\0';
    constexpr char tracerPidString[] = "TracerPid:";
    const auto tracer_pid_ptr = ::strstr(buf, tracerPidString);
    if (!tracer_pid_ptr) {
        return false; // Can't tell
    }

    for (const char *characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1; characterPtr <= buf + num_read;
         ++characterPtr) {
        if (::isspace(*characterPtr)) {
            continue;
        }
        return ::isdigit(*characterPtr) != 0 && *characterPtr != '0';
    }

    return false;
}

bool stopInDebugger() {
    if (amIBeingDebugged()) {
        __asm__("int $3");
        return true;
    }
    return false;
}

bool setCurrentThreadName(const std::string &name) {
    const size_t maxLen = 16 - 1; // Pthreads limits it to 16 bytes including trailing '\0'
    auto truncatedName = name.substr(0, maxLen);
    auto retCode = ::pthread_setname_np(::pthread_self(), truncatedName.c_str());
    return retCode == 0;
}

#endif
